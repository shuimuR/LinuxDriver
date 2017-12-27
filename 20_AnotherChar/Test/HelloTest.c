#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

void Print_Usage()
{
    printf("Hello <dev>\n");
}

int main(int argc, char **argv)
{
    int fd;

    if(argc != 2)
    {
        Print_Usage();
        return -1;
    }

    fd = open(argv[1], O_RDWR);
    if(fd < 0)
        printf("%s can't open\n", argv[1]);
    else
        printf("%s can open\n", argv[1]);

    return 0;
}
