#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>

#define SIZE 1 << 12

int main(void)
{
    int fd = open("/proc/maptest", O_RDONLY);
    if (fd == -1) {
        perror("open /proc/maptest error\n");
    }
    char* buf = (char*) mmap(NULL, SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
    //fputs(buf, stdout);
    printf("%s",buf);
    munmap(buf, SIZE);
    return 0;
}