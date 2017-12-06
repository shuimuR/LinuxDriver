#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int fd;
	int val = 1;
	fd = open("/dev/LED", O_RDWR);
	if(fd < 0)
		printf("Can't open!\n");
		
	if(argc != 2)
	{
		printf("Usage: %s <on|off>\n", argv[0]);
		return 0;
	}

	if((strcmp(argv[1], "on") == 0) || (strcmp(argv[1], "ON") == 0))
	{
		val = 1;	
	}
	else
		val = 0;
	write(fd, &val, 1);
	return 0;	
}
