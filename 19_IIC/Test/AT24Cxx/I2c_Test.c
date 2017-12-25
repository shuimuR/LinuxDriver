#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

void Print_Usage(char *file)
{
    printf("%s r addr\n", file);
    printf("%s w addr val\n", file);
}

int main(int argc, char **argv)
{
    unsigned char buf[2];
    int fd;

    if(argc !=3 && (argc != 4))
    {
        Print_Usage(argv[0]);
        return -1;
    }

    fd = open("/dev/AT24Cxx", O_RDWR);
    if(fd < 0)
    {
        printf("Can't open /dev/AT24Cxx\n");
        return -1;
    }

    if(strcmp(argv[1], "r") == 0)
    {
        buf[0] = strtoul(argv[2], NULL, 0);
        read(fd, buf, 1);
        printf("Data: %c, %d, 0x%2x\n", buf[0], buf[0], buf[0]);
    }
    else if(strcmp(argv[1], "w") == 0)
    {
        buf[0] = strtoul(argv[2], NULL, 0);
        buf[0] = strtoul(argv[3], NULL, 0);
        write(fd, buf ,2);
    }
    else
    {
        Print_Usage(argv[0]);
        return -1;
    }
}