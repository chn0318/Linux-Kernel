#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#define SCHED_FIFO 1
#define SCHED_RR 2
#define SCHED_WRR 6
void readcommand(int argc, char* argv[], long* repeat, int* policy, int* fork_time)
{
    if (argc != 4) 
    {
        fprintf(stderr, "Usage: time -p ./test_cpubound REPEAT_TIME SCHED_POLICY PROC_COUNT\n");
        exit(EXIT_FAILURE);
    }
   
    *repeat = atol(argv[1]);
    if (!strcmp(argv[2], "SCHED_WRR")){
            *policy = SCHED_WRR;
    }
    else if (!strcmp(argv[2], "SCHED_FIFO")){
        *policy = SCHED_FIFO;
    }
    else if (!strcmp(argv[2], "SCHED_RR")){
            *policy = SCHED_RR;
    }
    *fork_time = atoi(argv[3]);
     
}

void children_process(long repeat) 
{
    double a = 3.1415862;
    double b = 2.7135234;
    long i;
    for(i=0; i<repeat; i++){
        a=a*b;
        a=a/b;
        b=pow(b,2);
        b=sqrt(b);
    }

    return ;
}

int main(int argc, char* argv[])
{

    int i;
    int pid;
    long iterations;
    struct sched_param param;
    int policy;
    int fork_time;

    srand(time(0));
    readcommand(argc, argv, &iterations, &policy, &fork_time);
    if(policy!=SCHED_WRR){
       param.sched_priority = sched_get_priority_max(policy);
    }
    else{
        param.sched_priority=0;
    }
    printf("Current Scheduling Policy: %d\n", sched_getscheduler(0));
    printf("Setting Scheduling Policy to: %d\n", policy);

    if (sched_setscheduler(0, policy, &param))
    {
        perror("Error setting scheduler policy\n");
        exit(EXIT_FAILURE);
    }
    printf("New Scheduling Policy: %d\n", sched_getscheduler(0));

    int* children = malloc(sizeof(int) * fork_time);
    printf("Start forking children...\n");
    fflush(0); 

    for (i = 0; i < fork_time; i++)
    {
        pid = fork();
        if (pid > 0)
        {
            children[i] = pid;
        } 
        else if (pid == 0) 
        {
            children_process(iterations);
            _exit(0);
        } 
    }

    for (i = 0; i < fork_time; i++)
    {
        waitpid(children[i], NULL, 0);
    }
    free(children);

    return 0;
}

