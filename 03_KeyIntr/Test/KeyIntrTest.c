#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int fd;
	fd = open("/dev/key", O_RDWR);
	if(fd < 0)
		printf("Can't open!\n");
		
	unsigned char KeyVal= 0;
	while(1)
	{
		read(fd, &KeyVal, 1);
		printf("KeyVal = 0x%x\n", KeyVal);	
	}

	return 0;	
}
