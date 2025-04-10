#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	int status;
	pid_t pid=fork();
	
	if(pid==0) //자식 프로세스가 하는 행동
	{
		sleep(15);
		return 24; //15초 있다가 24 반환
	}
	else //부모 프로세스가 하는 행동
	{
		while(!waitpid(-1, &status, WNOHANG)) //모든 자식프로세스가 종료되기를 기다리는데 종료된 자식이 없어도 블로킹안하고 0 리턴
		{
			sleep(3);
			puts("sleep 3sec.");
		}

		if(WIFEXITED(status)) //종료된 프로세스의 status확인
			printf("Child send %d \n", WEXITSTATUS(status)); //종료되었으면 전달값 반환
	}
	return 0;
}

/*
root@my_linux:/home/swyoon/tcpip# gcc waitpid.c -o waitpid
root@my_linux:/home/swyoon/tcpip# ./waitpid
sleep 3sec.
sleep 3sec.
sleep 3sec.
sleep 3sec.
sleep 3sec.
Child send 24 
*/
