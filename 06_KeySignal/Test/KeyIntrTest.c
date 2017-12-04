#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

int fd;

void KeyValueGetFun(int signum)
{
	unsigned char KeyVal;
	read(fd, &KeyVal, 1);
	printf("The key value from signal: 0x%x\n", KeyVal);
}

int main(int argc, char **argv)
{
	int ret;
	int OFlags;

	signal(SIGIO, KeyValueGetFun);

	fd = open("/dev/key", O_RDWR);
	if(fd < 0)
		printf("Can't open!\n");
	
	fcntl(fd, F_SETOWN, getpid());

	OFlags = fcntl(fd, F_GETFL);

	fcntl(fd, F_SETFL, OFlags | FASYNC);

	while(1)
	{
		sleep(1000);
	}	

	return 0;	
}
