#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>

int main(int argc, char **argv)
{
	int fd;
	int ret;
	struct pollfd fds[1];

	fd = open("/dev/key", O_RDWR);
	if(fd < 0)
		printf("Can't open!\n");
	
	fds[0].fd = fd;
	fds[0].events = POLLIN;	
	unsigned char KeyVal= 0;
	while(1)
	{
		ret = poll(fds, 1, 5000);
		if(ret == 0)
		{
			printf("time out\n");
		}
		else
		{
			read(fd, &KeyVal, 1);
			printf("KeyVal = 0x%x\n", KeyVal);
		}	
	}	

	return 0;	
}
