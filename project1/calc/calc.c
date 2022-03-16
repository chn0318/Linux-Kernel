#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define MAX_SIZE 128
#define ID "<519021910480>"

static int operand1;
module_param(operand1, int, 0);
static char *operator;
module_param(operator, charp, 0);
static int operand2[MAX_SIZE];
static int ninp;
module_param_array(operand2, int, &ninp, 0);

static struct proc_dir_entry *proc_ent;
static struct proc_dir_entry *proc_dir;
static char output[MAX_SIZE];
int out_len;
static int getlenth(char* target){
    int i=0;
    while(output[i]!='\0'){
        i++;
    }
    return i;
}
static int chartoint(int length,char* buff){
    int value=0;
    int i=0;
    while(buff[i]>='0' && buff[i]<='9'){
        value=value*10+buff[i]-'0';
        i++;
    }
    return value;
}
static int inttochar(int index,char * buff,int val){
    int tmp=val;
    int i=0;
    char tmp_buf[20];
    if(tmp==0){
       buff[index]='0';
       index++;
       return index;
    }
    while(tmp>0){
       tmp_buf[i]=tmp%10+'0';
       tmp=(tmp-tmp%10)/10;
       i++;
    }
    int j=0;
    for(j=i-1;j>=0;j--){
    	buff[index]=tmp_buf[j];
    	index++;
    }
    return index;
}
static void calc(void){
    int i;
    for(i=0;i<MAX_SIZE;i++){
        output[i]='\0';
    }
    int index=0;
    if(operator[0]=='a'){
        for(i=0;i<ninp;i++){
           index=inttochar(index,output,operand2[i]+operand1);
            if(i!=ninp-1){
                output[index]=',';
                index++;
            }
        }
    }
    else{
        for(i=0;i<ninp;i++){
            index=inttochar(index,output,operand2[i]*operand1);
            if(i!=ninp-1){
                output[index]=',';
                index++;
            }
        }
    }
    output[index]='\0';
}
static ssize_t proc_read(struct file *fp, char __user *ubuf, size_t len, loff_t *pos)
{
    int count=0;
    if(*pos > 0) return 0;
    if(len < MAX_SIZE) return -EFAULT;
    pr_info("read\n");
    count=getlenth(output);
    output[count]='\n';
    count++;
    if(copy_to_user(ubuf,output,count)) return -EFAULT;
    *pos = count;
    return count;
}


static ssize_t proc_write(struct file *fp, const char __user *ubuf, size_t len, loff_t *pos)
{
    if(*pos > 0||len > MAX_SIZE) return -EFAULT;
    char buffer[10];
    if(copy_from_user(buffer,ubuf,len)) return -EFAULT;
    output[len+1] = '\0';
    operand1=chartoint(len,buffer);
    calc();
    pr_info("write: operand1= %d\n",operand1);
    return len;
}

static const struct proc_ops proc_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

static int __init proc_init(void)
{
    proc_dir= proc_mkdir(ID,NULL);
    proc_ent = proc_create("calc", 0666, proc_dir, &proc_ops);
    if (!proc_ent)
    {
        proc_remove(proc_ent);
        pr_alert("Error: Could not initialize /proc/<519021910480>\n");
        return -ENOMEM;
    }
    calc();
    pr_info("/proc/<519021910480>/calc created\n");
    return 0;
}

static void __exit proc_exit(void)
{
    proc_remove(proc_ent);
    proc_remove(proc_dir);
    pr_info("/proc/<519021910480>/calc removed\n");
}

module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");
