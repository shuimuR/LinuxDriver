/******************************************
usage
./RegEditorTest r8 addr [num]
./RegEditorTest r16 addr [num]
./RegEditorTest r32 addr [num]

./RegEditorTest w8 addr [num]
./RegEditorTest w16 addr [num]
./RegEditorTest w32 addr [num]


******************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define KER_RW_R8   0
#define KER_RW_R16   1
#define KER_RW_R32   2

#define KER_RW_W8   3
#define KER_RW_W16   4
#define KER_RW_W32   5

void PrintUsage(char *file)
{
    printf("Usage:\n");
    printf("%s <r8 | r16 | r32> <phy addr> [num]\n", file);
    printf("%s <w8 | w16 | w32> <phy addr> <value>\n", file);
}

int main(char argc, char **argv)
{
    int fd;
    unsigned int buf[2];
    unsigned int num;
    unsigned int i;

    if(argc != 3 && argc != 4)
    {
        PrintUsage(argv[0]);
        return -1;
    }

    fd = open("/dev/RegEdit", O_RDWR);
    if(fd < 0)
    {
        printf("Open /dev/RegEdit error\n");
        return -2;
    }

    //address
    buf[0] = strtol(argv[2], NULL, 0);

    if(argc == 4)
    {
        buf[1] = strtol(argv[3], NULL, 0);
        num = buf[1];
    }
    else
    {
        num = 1;
    }

    if(strcmp(argv[1], "r8") == 0)
    {
        for(i = 0; i < num; i++)
        {
            ioctl(fd, KER_RW_R8, buf);
            printf("[%08x] = %02x.\n", buf[0], buf[1]);
            buf[0]++;
        }
    }
    else if(strcmp(argv[1], "r16") == 0)
    {
        for(i = 0; i < num; i++)
        {
            ioctl(fd, KER_RW_R16, buf);
            printf("[%08x] = %02x.\n", buf[0], buf[1]);
            buf[0]+= 2;
        }
    }
    else if(strcmp(argv[1], "r32") == 0)
    {
        for(i = 0; i < num; i++)
        {
            ioctl(fd, KER_RW_R32, buf);
            printf("[%08x] = %02x.\n", buf[0], buf[1]);
            buf[0]+= 4;
        }
    }
    else if(strcmp(argv[1], "w8") == 0)
    {
        ioctl(fd, KER_RW_W8, buf);
    }
    else if(strcmp(argv[1], "w16") == 0)
    {
        ioctl(fd, KER_RW_W16, buf);
    }
    else if(strcmp(argv[1], "w32") == 0)
    {
        ioctl(fd, KER_RW_W32, buf);
    }
    else
        PrintUsage(argv[0]);
    return 0;
}
