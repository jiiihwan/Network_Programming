#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void timeout(int sig)
{
	if(sig==SIGALRM)
		puts("Time out!");
	alarm(2);	
}

int main(int argc, char *argv[])
{
	int i;
	struct sigaction act; //시그널 처리 정보를 담고 있는 sigaction 구조체 변수 act선언
	act.sa_handler=timeout; //시그널 정보를 전달
	sigemptyset(&act.sa_mask); //sa_mask 모든 비트 0으로 초기화
	act.sa_flags=0; //sa_flags 도 0으로 초기화
	sigaction(SIGALRM, &act, 0); //설정한 핸들러를 SIGALRM 시그널에 등록

	alarm(2);

	for(i=0; i<3; i++)
	{
		puts("wait...");
		sleep(100);
	}
	return 0;
}