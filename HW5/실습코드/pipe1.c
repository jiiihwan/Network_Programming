#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#define BUF_SIZE 30

int main(int argc, char *argv[])
{
	int fds[2];
	char str[]="Who are you?";
	char buf[BUF_SIZE];
	pid_t pid;
	
	pipe(fds); // fds[0]: 읽기용, fds[1]: 쓰기용
	pid=fork();
	if(pid==0) //자식프로세스
	{
		write(fds[1], str, sizeof(str)); 
	}
	else //부모프로세스
	{
		read(fds[0], buf, BUF_SIZE);
		puts(buf);
	}
	return 0;
}