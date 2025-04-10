#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	int status;
	pid_t pid=fork(); //fork 하면 현재 프로세스 복사
	
	if(pid==0) //자식 프로세스이면
	{
		return 3;
	}
	else //부모 프로세스이면
	{
		printf("Child PID: %d \n", pid);
		pid=fork();
		if(pid==0)
		{
			exit(7);
		}
		else
		{
			printf("Child PID: %d \n", pid);
			wait(&status);
			printf("status: %d\n", status);
			if(WIFEXITED(status))
				printf("Child send one: %d \n", WEXITSTATUS(status));

			wait(&status);
			printf("status: %d\n", status);
			if(WIFEXITED(status))
				printf("Child send two: %d \n", WEXITSTATUS(status));
			sleep(30);     // Sleep 30 sec.
		}
	}
	return 0;
}