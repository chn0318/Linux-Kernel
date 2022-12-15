#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/highmem.h>

#define MAX_SIZE 128

static struct proc_dir_entry *proc_ent;
static char output[MAX_SIZE];
static int out_len;

enum operation {
    OP_READ, OP_WRITE
};
unsigned long* get_addr(struct task_struct* task,unsigned long addr){
    struct mm_struct* mm;
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    mm=task->mm;
    pgd=mm->pgd;
    pgd+=pgd_index(addr);
    if(pgd_none(*pgd)||pgd_bad(*pgd)){
        pr_info("Find PGD error");
        return -1;
    }
    p4d =p4d_offset(pgd,addr);
    if(p4d_none(*p4d)||p4d_bad(*p4d)){
        pr_info("Find P4D error");
        return -1;
    }
    pud = pud_offset(p4d,addr);
    if(pud_none(*pud)||pud_bad(*pud)){
        pr_info("Find PUD error");
        return -1;
    }
    pmd = pmd_offset(pud,addr);
    if(pmd_none(*pmd)||pmd_bad(*pmd)){
        pr_info("Find PMD error");
        return -1;
    }
    
    if(!(pte = pte_offset_kernel(pmd,addr))){
        pr_info("Find PTD error");
        return -1;
    }
    struct page *page;
    page = pte_page(*pte);
    unsigned long *kernel_addr;
    kernel_addr = (unsigned long*)kmap_local_page(page);
    pr_info("kernel base addr = 0x%lx",kernel_addr);
    //kernel_addr = (unsigned long*)page_address(page);
    //pr_info("kernel base addr = 0x%lx",kernel_addr);
    unsigned long kernel_base = kernel_addr;
    kernel_base = kernel_base & PAGE_MASK;
    unsigned long kernel_offset = addr & (~PAGE_MASK);
    kernel_addr = kernel_base |kernel_offset;    
    pr_info("base = 0x%lx,offset = 0x%lx, kernel addr = 0x%lx",kernel_base,kernel_offset,kernel_addr);
    return kernel_addr;
}

static ssize_t proc_read(struct file *fp, char __user *ubuf, size_t len, loff_t *pos)
{
    int count; /* the number of characters to be copied */
    out_len=strlen(output);
    if (out_len - *pos > len) {
        count = len;
    }
    else {
        count = out_len - *pos;
    }
    if (copy_to_user(ubuf, output, count)) {
        return -EFAULT;
    }
    *pos += count;
    
    return count;

}

static ssize_t proc_write(struct file *fp, const char __user *ubuf, size_t len, loff_t *pos)
{
    // TODO: parse the input, read/write process' memory
    //step1: copy the input from userspace to kernel
    if(*pos > 0||len > MAX_SIZE) return -EFAULT;
    char tmp_buffer[MAX_SIZE];
    char phase_buffer[MAX_SIZE];
    if(copy_from_user(tmp_buffer,ubuf,len)) return -EFAULT;
    tmp_buffer[len-1]=' ';
    tmp_buffer[len]='\0';
    pr_info("the command :%s,the length=%d\n",tmp_buffer,strlen(tmp_buffer));
    //step2: phase the command
    char *token;
    char *iter = strcpy(phase_buffer,tmp_buffer);
    char delim[]=" ";
    token = strsep(&iter,delim);
    if(strcmp(token,"r")==0){
        //step3.1: read the value from the specified address
        unsigned long target_pid;
        unsigned long addr;
        int count=0;
        while(token=strsep(&iter,delim)){
            if(strcmp(token,"")==0){
                continue;
            }
            else if(count==0){
                kstrtoul(token, 0, &target_pid);
                pr_info("pid=%ld",target_pid);
                count++;
            }
            else if(count==1){
                char tmp[MAX_SIZE]="0x";
                strcat(tmp,token);
                kstrtoul(tmp,0,&addr);
                pr_info("address=%lx",addr);
                count++;
            }
        }
        //get the task_struct by pid
        struct task_struct *target_task;
        target_task=pid_task(find_get_pid(target_pid),PIDTYPE_PID);
        //get the vaddr in kernel space
        unsigned long *vaddr;
        vaddr=get_addr(target_task,addr);
        output[0]=*vaddr;
        output[1]='\0';
    }
    else if (strcmp(token,"w")==0){
        //step 3.2: write the value to the specified address
        unsigned long target_pid;
        unsigned long addr;
        unsigned long content;
        int count=0;
        while(token=strsep(&iter,delim)){
            if(strcmp(token,"")==0){
                continue;
            }
            else if(count==0){
                kstrtoul(token, 0, &target_pid);
                pr_info("pid=%ld",target_pid);
                count++;
            }
            else if(count==1){
                char tmp[MAX_SIZE]="0x";
                strcat(tmp,token);
                kstrtoul(tmp,0,&addr);
                pr_info("address=%lx",addr);
                count++;
            }
            else if(count==2){
                kstrtoul(token,0,&content);
                pr_info("content=%ld",content);
                count++;
            }
        }
        struct task_struct *target_task;
        target_task=pid_task(find_get_pid(target_pid),PIDTYPE_PID);
        //get the vaddr in kernel space
        unsigned long *vaddr;
        vaddr=get_addr(target_task,addr);
        *vaddr=(char)content;

    }
    else{
        pr_info("unable to phase the command\n");
    }
    return len+1;
}

static const struct proc_ops proc_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

static int __init mtest_init(void)
{
    proc_ent = proc_create("mtest", 0666, NULL, &proc_ops);
    if (!proc_ent)
    {
        proc_remove(proc_ent);
        pr_alert("Error: Could not initialize /proc/mtest\n");
        return -EFAULT;
    }
    pr_info("/proc/mtest created\n");
    return 0;
}

static void __exit mtest_exit(void)
{
    proc_remove(proc_ent);
    pr_info("/proc/mtest removed\n");
}

module_init(mtest_init);
module_exit(mtest_exit);
MODULE_LICENSE("GPL");