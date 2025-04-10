#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	int status;
	pid_t pid=fork(); //fork 하면 현재 프로세스 복사
	
	if(pid==0) //자식 프로세스가 하는 행동
	{
		return 3; //자식프로세스가 3을 반환하고 종료
	}
	else //부모 프로세스가 하는 행동
	{
		printf("Child PID: %d \n", pid);
		pid=fork(); //부모 프로세스에서 다시 fork
		if(pid==0) //자식프로세스가 하는 행동
		{
			exit(7); //7을 반환하고 종료
		}
		else //부모프로세스가 하는 행동
		{
			printf("Child PID: %d \n", pid); 
			wait(&status); //자식이 종료될때까지 block
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