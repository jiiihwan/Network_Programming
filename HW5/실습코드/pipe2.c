#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#define BUF_SIZE 30

int main(int argc, char *argv[])
{
	int fds[2];
	char str1[]="Who are you?";
	char str2[]="Thank you for your message";
	char buf[BUF_SIZE];
	pid_t pid;

	pipe(fds);
	pid=fork();

	if(pid==0) //자식프로세스
	{
		write(fds[1], str1, sizeof(str1)); //1. str1을 wrtie
		sleep(2); //자식프로세스가 부모프로세스의 write를 늦게 읽기 위한 딜레이
		read(fds[0], buf, BUF_SIZE); //4. buf에 str2를 읽기
		printf("Child proc output: %s \n",  buf); //5. buf출력
	}
	else //부모프로세스
	{
		read(fds[0], buf, BUF_SIZE); //2. buf에 str1을 읽기
		printf("Parent proc output: %s \n", buf); //buf출력
		write(fds[1], str2, sizeof(str2)); //3. str2를 write
		sleep(3); //너무빨리 if문 탈출하지 않도록 방지
	}
	return 0;
}
