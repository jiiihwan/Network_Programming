#include <stdio.h>
#include <sys/uio.h>
#define BUF_SIZE 100

int main(int argc, char *argv[])
{
	struct iovec vec[2]; //두개의 버퍼
	char buf1[BUF_SIZE]={0,};
	char buf2[BUF_SIZE]={0,};
	int str_len;

	vec[0].iov_base=buf1; //첫 5바이트는 buf1에
	vec[0].iov_len=5; 
	vec[1].iov_base=buf2; //나머지는 buf2에
	vec[1].iov_len=BUF_SIZE;

	str_len=readv(0, vec, 2); //stdin(0)으로부터 읽음
	printf("Read bytes: %d \n", str_len);
	printf("First message: %s \n", buf1);
	printf("Second message: %s \n", buf2);
	return 0;
}

/*
swyoon@my_linux:~/tcpip$ gcc readv.c -o rv
swyoon@my_linux:~/tcpip$ ./rv
I like TCP/IP socket programming~
Read bytes: 34 
First message: I lik 
Second message: e TCP/IP socket programming~
*/

