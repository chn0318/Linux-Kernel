#include <stdio.h>
#include <unistd.h>

int main(void)
{
    char c = 42;
    pid_t pid = getpid();
    printf("pid: %d\n", pid);
    printf("addr: %lx\n", (unsigned long)(&c));
    printf("content: %d\n", (int)c);

    FILE* fp = fopen("/proc/mtest", "w");
    fprintf(fp, "r %d %lx\n", pid, (unsigned long)(&c));
    fclose(fp);
    fp = fopen("/proc/mtest", "r");
    char r;
    fscanf(fp, "%c", &r);
    printf("read result: %d\n", (int)r);
    fclose(fp);

    fp = fopen("/proc/mtest", "w");
    fprintf(fp, "w %d %lx 1\n", pid, (unsigned long)(&c));
    fclose(fp);
    printf("content: %d\n", (int)c);
    return 0;
}