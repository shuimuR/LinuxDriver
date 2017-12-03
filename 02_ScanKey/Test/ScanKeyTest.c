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
		
	unsigned char KeyVals[4] = {0};
	unsigned int KeyPressCount = 0;
	unsigned long KeyFlag;
	while(1)
	{
		read(fd, KeyVals, sizeof(KeyVals));
		KeyFlag = KeyVals[0] +  KeyVals[1] + KeyVals[2] + KeyVals[3];
		if(KeyFlag != 0)
		{
			printf("Button0: %s\n", KeyVals[0] ? "Pressed": "Released");
			printf("Button1: %s\n", KeyVals[1] ? "Pressed": "Released");
			printf("Button2: %s\n", KeyVals[2] ? "Pressed": "Released");
			printf("Button3: %s\n", KeyVals[3] ? "Pressed": "Released");
		}
	}

	return 0;	
}
