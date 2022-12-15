#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h> 
#include <sched.h> 
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <linux/unistd.h>
#include <linux/sched.h>
int main() {
    int  policy,buf;
    struct timespec wrr_time;
    pid_t pid;
    printf("Input the process id (PID) you want to check: ");
    scanf("%d", &buf);
    pid = buf;
    if (!pid) {
        perror("Invalid PID. Aborting...");
        exit(-1);
    }
    sched_rr_get_interval(pid,&wrr_time);
    policy = sched_getscheduler(pid);
    printf("Schedule policy: ");
    switch(policy)
	{
		case 0: printf("normal \n"); break;
		case 1: printf("fifo \n"); break;
		case 2: printf("rr \n"); break;
		case 3: printf("batch \n"); break;
		case 5: printf("idle \n"); break;
		case 6: printf("wrr \n"); break;
	}
    buf = wrr_time.tv_nsec/1000000;
    printf("Timeslice: %d milisec\n", buf);
}