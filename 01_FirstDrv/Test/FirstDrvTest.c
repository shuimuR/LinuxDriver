#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int fd;
	int val = 1;
	fd = open("/dev/xxx", O_RDWR);
	if(fd < 0)
		printf("Can't open!\n");
		
	write(fd, &val, 1);
	return 0;	
}
