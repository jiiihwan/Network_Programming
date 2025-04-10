#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void timeout(int sig) //SIGALRM 시그널 발생했을때 time out출력
{
	if(sig==SIGALRM) 
		puts("Time out!");

	alarm(2); //2초뒤에 SIGALRM 시그널 발생
}
void keycontrol(int sig) //ctrl+c 가 입력되면 문자 출력
{
	if(sig==SIGINT)
		puts("CTRL+C pressed");
}

int main(int argc, char *argv[])
{
	int i;
	signal(SIGALRM, timeout); //SIGALRM 발생시 timeout 함수 호출
	signal(SIGINT, keycontrol); //SIGINT 발생시 keycontrol 함수 호출
	alarm(2); //2초뒤에 SIGALRM 시그널 발생

	for(i=0; i<3; i++) //3번만 반복
	{
		puts("wait...");
		sleep(100); //2초뒤에 오는 SIGALRM 을 기다리기 위해 설정
	}
	return 0;
}