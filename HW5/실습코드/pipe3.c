#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#define BUF_SIZE 30

int main(int argc, char *argv[])
{
	int fds1[2], fds2[2]; //파이프 두개
	char str1[]="Who are you?";
	char str2[]="Thank you for your message";
	char buf[BUF_SIZE];
	pid_t pid;
	
	pipe(fds1), pipe(fds2);
	pid=fork();
	
	if(pid==0) //자식 프로세스
	{
		write(fds1[1], str1, sizeof(str1)); //fds1 으로 str1을 write
		read(fds2[0], buf, BUF_SIZE); //fds2로 buf에 읽기
		printf("Child proc output: %s \n",  buf); //buf출력
	}
	else //부모 프로세스
	{
		read(fds1[0], buf, BUF_SIZE); //fds1으로 buf에 읽기
		printf("Parent proc output: %s \n", buf); //buf출력
		write(fds2[1], str2, sizeof(str2)); //fds2로 str2 write
		sleep(3);
	}
	return 0;
}