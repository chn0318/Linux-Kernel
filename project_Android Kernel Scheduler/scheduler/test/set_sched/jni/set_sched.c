#define _GNU_SOURCE
#include <linux/sched.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <linux/unistd.h>
#include <sys/time.h> 
#include <sched.h> 
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#define SCHED_NORMAL    0
#define SCHED_FIFO      1
#define SCHED_RR        2
#define SCHED_BATCH     3
#define SCHED_IDLE      5
#define SCHED_WRR       6
int main() {
    int ret, buf;
    struct sched_param param;
    pid_t pid;
    int policy,oldpolicy;
    printf("Input the process id (PID) you want to modify: ");
    scanf("%d", &buf);
    pid = buf;
   
    if (!pid) {
        perror("Invalid PID. Aborting...");
        exit(-1);
    }
    printf("Input the schedule policy you want to change (0-NORMAL, 1-FIFO, 2-RR, 6-WRR):\n");
    scanf("%d", &policy);

    if(policy != 0 && policy != 1 && policy!= 2&& policy!=6)
    {
        perror("Wrong schedule policy. Aborting...");
        exit(-1);
    }

    printf("Set process's priority: ");
    scanf("%d",&buf);
    if(policy!=6)
        param.sched_priority = buf;
    else
        param.sched_priority = 0;

    printf("Changing Scheduler for PID %d\n", pid);


    ret = sched_setscheduler(pid, policy, &param);
    if (ret < 0) {
        perror("Changing scheduler failed. Aborting...");
        exit(-1);
    }
    printf("Switch finish!\n");
}