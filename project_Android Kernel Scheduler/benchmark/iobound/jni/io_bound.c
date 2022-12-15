#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define SCHED_FIFO 1
#define SCHED_RR 2
#define SCHED_WRR 6
#define USAGE "Usage: time -p ./test_iobound SCHED_POLICY /data/misc/data_in /data/misc/data_out  BLOCK_SIZE TRAN_SIZE PROC_COUNT\n"

void children_process(size_t, size_t, char*, char*);
void readcommand(int, char**, int*, int*, size_t*, ssize_t*, FILE*);

int main(int argc, char* argv[])
{

    int i;
    int pid;
    struct sched_param param;
    int policy;
    int fork_count;
    size_t block_size;
    ssize_t tran_size;
    int file_counter = 0;
    char dest_name[100];
    FILE* src;

    /* Parge command line */
    readcommand(argc, argv, &policy, &fork_count, &block_size, &tran_size, src);

    
    if(policy!=SCHED_WRR){
        //if the policy is not wrr, then set the max priority of the policy
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
        perror("Error setting scheduler policy");
        exit(EXIT_FAILURE);
    }
    printf("New Scheduling Policy: %d\n", sched_getscheduler(0));
    int* children = malloc(sizeof(int) * fork_count);
    printf("Starting forking children...\n");
    fflush(0); 

    for (i = 0; i < fork_count; i++)
    {
        pid = fork();
        if (pid > 0)
        {
            file_counter += 1;
            children[i] = pid;
        } 
        else if (pid == 0) 
        {
            children_process(block_size, tran_size, argv[2], argv[3]);
            _exit(0);
        } 
    }

    for (i = 0; i < fork_count; i++)
    {
        waitpid(children[i], NULL, 0);
    }
    free(children);

    return 0;
}

/* get the argument from the command line  */
void readcommand(int argc, char* argv[], int* policy, int* child_count, 
                        size_t* block_size, ssize_t* tran_size, FILE* src)
{
    if (argc < 7)
    {
        fprintf(stderr, "Not enough arguments: %d\n", argc - 1);
        fprintf(stderr, USAGE);
        exit(EXIT_FAILURE);
    }
    /*get the policy*/
    if (!strcmp(argv[1], "SCHED_WRR"))
    {
        *policy = SCHED_WRR;
    } 
    else if (!strcmp(argv[1], "SCHED_FIFO"))
    {
        *policy = SCHED_FIFO;
    } 
    else if (!strcmp(argv[1], "SCHED_RR"))
    {
        *policy = SCHED_RR;
    } 
    else
    {
        fprintf(stderr, "Unhandled scheduling policy\n");
        exit(EXIT_FAILURE);
    }

    /* Test that you can open the src file */
    src = fopen(argv[2], "r");
    if (src == NULL) 
    {
        fprintf(stderr, "There was an error opening the source file: %s\n", argv[2]);
        perror("");
        exit(EXIT_FAILURE);
    }
    fclose(src);
    *block_size = atoi(argv[4]);
    *tran_size = atoi(argv[5]);
    *child_count = atoi(argv[6]);
}

void children_process(size_t b_size, size_t t_size, char* src_name, char* dest_name) 
{
    int srcFD;
    int destFD;
    ssize_t bytes_read = 0;
    ssize_t bytes_written = 0;
    ssize_t total_bytes_written = 0;
    char* buf = NULL;
    /* Open source file. */
    srcFD = open(src_name, O_RDONLY);
    /* Open/create dest file. */
    destFD = open(dest_name, O_WRONLY | O_CREAT );
    /* malloc buffer */
    buf = malloc(b_size * sizeof(char));
    do {
        bytes_read = read(srcFD, buf, b_size);
        if (bytes_read < 0)
        {
            perror("Error reading source file.");
            exit(EXIT_FAILURE);
        }

        if (bytes_read > 0) 
        {
            bytes_written = write(destFD, buf, bytes_read);           
            total_bytes_written += bytes_written;
            
        }
        if (bytes_read != (ssize_t)b_size)
        {
            if (lseek(srcFD, 0, SEEK_SET))
            {
                perror("Error seeking to beginning of file.");
                exit(EXIT_FAILURE);
            }
        }
    } while (total_bytes_written < (ssize_t)t_size);
    close(srcFD);
    close(destFD);
    free(buf);
}
