#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#define SCHED_FIFO 1
#define SCHED_RR 2
#define SCHED_WRR 6
#define MIN_ITERATIONS 100
#define USAGE 


void readcommand(int argc, char* argv[], long* iterations, int* policy, int* fork_time, 
                        size_t* block_size, ssize_t* tran_size)
{
    if (argc < 8)
    {
        fprintf(stderr,"Usage: time -p ./mix_test REPEAT SCHED_POLICY FORK_TIME BLOCK_SIZE TRAN_SIZE SRC DEST \n");
        exit(EXIT_FAILURE);
    }

    /* Set iterations if supplied */
    *iterations = atol(argv[1]);
    /* Set policy if supplied */
    if (!strcmp(argv[2], "SCHED_WRR"))
    {
        *policy = SCHED_WRR;
    }
    else if (!strcmp(argv[2], "SCHED_FIFO"))
    {
        *policy = SCHED_FIFO;
    }
    else if (!strcmp(argv[2], "SCHED_RR"))
    {
        *policy = SCHED_RR;
    }

    *fork_time = atoi(argv[3]);
    *block_size = atoi(argv[4]);
    *tran_size = atoi(argv[5]);
    
}
double cpubound(long iter) 
{
   double a = 3.1415862;
    double b = 2.7135234;
   long i;
    for( i=0; i<iter; i++){
        a=a*b;
        a=a/b;
        b=pow(b,2);
        b=sqrt(b);
    }
    return b;
}

void children_process(long repeat, char* src, char* dest, size_t b_size, size_t t_size){
    int i;

    for (i = 0; i < MIN_ITERATIONS; i++)
    { 
        cpubound(repeat / MIN_ITERATIONS);
        iobound(b_size, t_size / MIN_ITERATIONS, src, dest);
    }
}


void iobound(size_t b_size, ssize_t t_size, char* src_name, char* dest_name) 
{  
    ssize_t read_size = 0;
    ssize_t write_size = 0;
    ssize_t sum = 0;
    char* buffer = NULL;
    int source_file;
    int dest_file;
    source_file = open(src_name, O_RDONLY);
    dest_file = open(dest_name, O_WRONLY | O_CREAT);
    buffer = malloc(b_size * sizeof(char));
    do 
    {
        read_size = read(source_file, buffer, b_size);
        write_size = write(dest_file, buffer, read_size);
        sum += write_size;

    } while(sum < (ssize_t)t_size);
    close(source_file);
    close(dest_file);  
    free(buffer);
}
int main(int argc, char* argv[]){

    int i;
    int pid;
    long iterations;
    struct sched_param param;
    int policy;
    int fork_time;
    size_t block_size = 0;
    ssize_t tran_size = 0;
    readcommand(argc, argv, &iterations, &policy, &fork_time, &block_size, &tran_size);
    if(policy!=SCHED_WRR){
        param.sched_priority = sched_get_priority_max(policy);
    }
    else{
        param.sched_priority = 0;
    }
    /* Set new scheduler policy */
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
            children_process(iterations, argv[6], argv[7], block_size, tran_size);
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

