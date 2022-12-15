#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/sched/cputime.h>
#include <linux/mm.h>

#define MAX_SIZE 128
#define Access_MASK 1 << 5      //pte的access位
static struct proc_dir_entry *proc_ent;
static char output[MAX_SIZE];
static int out_len;
static int pid_measured;
static struct task_struct *taskp;
static u64 nsecc_to_clock_t(u64 x) //单位转换：nsec->clock tick
{
#if (NSEC_PER_SEC % USER_HZ) == 0
	return div_u64(x, NSEC_PER_SEC / USER_HZ);
#elif (USER_HZ % 512) == 0
	return div_u64(x * USER_HZ / 512, NSEC_PER_SEC / 512);
#else

	return div_u64(x * 9, (9ull * NSEC_PER_SEC + (USER_HZ / 2)) / USER_HZ);
#endif
}

static void int2char(unsigned long long memory_count, int pos)
{
    if (memory_count == 0)
    {
        output[pos] = '0';
        output[pos + 1] = ' ';
        output[pos + 2] = '\0';
        return;
    }
    int memory_index = 0;
    char tmp_output[MAX_SIZE];
    unsigned long long tmp = memory_count;
    while (tmp != 0)
    {
        tmp_output[memory_index] = tmp % 10 + '0';
        memory_index += 1;
        tmp = tmp / 10;
    }
    int i = 0;
    for (i = 0; i < memory_index; i++)
    {
        output[i + pos] = tmp_output[memory_index - 1 - i];
    }
    output[memory_index + pos] = ' ';
    output[memory_index + pos + 1] = '\0';
}
static void cpu_measure(struct task_struct *taskp, unsigned long long *utime, unsigned long long *stime, unsigned long long *cutime, unsigned long long *cstime)
{
    unsigned long flags;
    struct task_struct *t;
    struct signal_struct *sig = taskp->signal;
    *cutime = sig->cutime;
    *cstime = sig->cstime;
    struct task_cputime cputime;
    unsigned int seq, nextseq;
    rcu_read_lock();
    nextseq = 0;
    do
    {
        seq = nextseq;
        flags = read_seqbegin_or_lock_irqsave(&sig->stats_lock, &seq);
        *utime += sig->utime;
        *stime += sig->stime;
        for_each_thread(taskp, t)   //遍历线程组的其他线程
        {
            *utime += t->utime;
            *stime += t->stime;
        }
        nextseq = 1;
    } while (need_seqretry(&sig->stats_lock, seq));
    done_seqretry_irqrestore(&sig->stats_lock, seq, flags);
    rcu_read_unlock();
}
static int ram_measure(struct task_struct *taskp)
{
    int page_count = 0;
    struct mm_struct *mm;
    struct vm_area_struct *vma;
    unsigned long long KERNEL_SPACE = 0XFFFF800000000000;  
    for (vma = taskp->mm->mmap; vma; vma = vma->vm_next)             //遍历链表中的每一个vm_area_struct
    {
        int page_number = (vma->vm_end - vma->vm_start) / PAGE_SIZE; //遍历每一个vm_area_struct中的page
        int i = 0;
        for (i = 0; i < page_number; i++)
        {
            unsigned long virt_addr = vma->vm_start + i * PAGE_SIZE;
            if (virt_addr >= KERNEL_SPACE)
            {
                pr_info("--------kernel space: %lx--------",virt_addr); //跳过进程的内核空间
                continue;
            }
            //进行地址转换
            pgd_t *pgd;
            p4d_t *p4d;
            pud_t *pud;
            pmd_t *pmd;
            pte_t *pte;
            pgd = taskp->mm->pgd;
            pgd += pgd_index(virt_addr);
            if (pgd_none(*pgd) || pgd_bad(*pgd))
            {
                pr_info("Find PGD error");
                continue;
            }
            p4d = p4d_offset(pgd, virt_addr);
            if (p4d_none(*p4d) || p4d_bad(*p4d))
            {
                pr_info("Find P4D error");
                continue;
            }
            pud = pud_offset(p4d, virt_addr);
            if (pud_none(*pud) || pud_bad(*pud))
            {
                pr_info("Find PUD error");
                continue;
            }
            pmd = pmd_offset(pud, virt_addr);
            if (pmd_none(*pmd) || pmd_bad(*pmd))
            {
                pr_info("Find PMD error");
                continue;
            }

            if (!(pte = pte_offset_kernel(pmd, virt_addr)))
            {
                pr_info("Find PTD error");
                continue;
            }
            //获得pte的Access位，并将其清零
            int flag = 0;
            unsigned long MASK = 1 << 5;
            flag = !!((pte->pte) & MASK);
            pte->pte = pte->pte & ~MASK;
            if (flag)
            {
                page_count += 1;
            }
        }
    }
    return page_count;
}
static ssize_t proc_read(struct file *fp, char __user *ubuf, size_t len, loff_t *pos)
{
    int count; /* the number of characters to be copied */
    int memory_count = 0;
    if (*pos == 0)
    {
        pr_info("a new read, update process' status");
        /* a new read, update process' status */
        if (find_get_pid(pid_measured))
        {
            memory_count = ram_measure(taskp);
            pr_info("The page accessed = %d\n", memory_count);
            int2char(memory_count, 0);
            out_len = strlen(output);
            unsigned long long utime, stime, cutime, cstime;
            utime = stime = cutime = cstime = 0;
            cpu_measure(taskp, &utime, &stime, &cutime, &cstime);
            utime = nsecc_to_clock_t(utime);
            stime = nsecc_to_clock_t(stime);
            cutime = nsecc_to_clock_t(cutime);
            cstime = nsecc_to_clock_t(cstime);
            int2char(utime, out_len);
            out_len = strlen(output);
            int2char(stime, out_len);
            out_len = strlen(output);
            int2char(cutime, out_len);
            out_len = strlen(output);
            int2char(cstime, out_len);
            out_len = strlen(output);
            pr_info("utime = %lld,stime = %lld,cutime = %lld,cstime = %lld\n", utime, stime, cutime, cstime);
        }
        else
        {  //当进程终止后，返回-1
            char tmp[20] = "-1 -1 -1 -1 -1 ";
            strcpy(output, tmp);
            out_len = strlen(output);
        }
    }
    if (out_len - *pos > len)
    {
        count = len;
    }
    else
    {
        count = out_len - *pos;
    }

    pr_info("Reading the proc file\n");

    if (copy_to_user(ubuf, output + *pos, count))
        return -EFAULT;
    *pos += count;
    return count;
}

static ssize_t proc_write(struct file *fp, const char __user *ubuf, size_t len, loff_t *pos)
{
    int pid;

    if (*pos > 0)
        return -EFAULT;
    pr_info("Writing the proc file\n");
    if (kstrtoint_from_user(ubuf, len, 10, &pid))
        return -EFAULT;

    taskp = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
    pid_measured = pid;
    if (find_get_pid(pid))
    {
        ram_measure(taskp);
        // CPU占有率的计算
        unsigned long long utime, stime, cutime, cstime;
        cutime = cstime = utime = stime = 0;
        cpu_measure(taskp, &utime, &stime, &cutime, &cstime);
        pr_info("utime = %lld,stime = %lld,cutime = %lld,cstime = %lld", utime, stime, cutime, cstime);
    }
    else{
        pr_info("Invailed pid\n");
    }
    *pos += len;
    return len;
}

static const struct proc_ops proc_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

static int __init watch_init(void)
{
    proc_ent = proc_create("watch", 0666, NULL, &proc_ops);
    if (!proc_ent)
    {
        proc_remove(proc_ent);
        pr_alert("Error: Could not initialize /proc/watch\n");
        return -EFAULT;
    }
    pr_info("/proc/watch created\n");
    return 0;
}

static void __exit watch_exit(void)
{
    proc_remove(proc_ent);
    pr_info("/proc/watch removed\n");
}

module_init(watch_init);
module_exit(watch_exit);
MODULE_LICENSE("GPL");
