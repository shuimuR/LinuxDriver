#include <stdio.h>
#include <signal.h>

void MySignalFun(int signum)
{
	static int cnt = 1;
	printf("signal = %d, %d times\n", signum, ++cnt); 	
}

int main(int argc, char **argv)
{
	signal(SIGUSR1, MySignalFun);
	
	while(1)
	{
		sleep(1000);	
	}
	return 0;
}
