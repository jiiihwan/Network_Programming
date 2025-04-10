#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *message);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);

int main(int argc, char *argv[])
{
	int sock;
	pid_t pid;
	char buf[BUF_SIZE];
	struct sockaddr_in serv_adr;
	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);   //TCP소켓 생성
	memset(&serv_adr, 0, sizeof(serv_adr)); //서버주소 초기화
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1) 
		error_handling("connect() error!");

	pid=fork();
	if(pid==0) //자식프로세스는
		write_routine(sock, buf); //쓰기 담당
	else //부모프로세스는 
		read_routine(sock, buf); //읽기 담당 

	close(sock);
	return 0;
}

void read_routine(int sock, char *buf)
{
	while(1)
	{
		int str_len=read(sock, buf, BUF_SIZE); //서버로부터 수신
		if(str_len==0) //수신길이가 0(서버 종료)이면 루프 끝
			return;

		buf[str_len]=0; //출력명령 전에 널 문자 수동으로 넣어주기
		printf("Message from server: %s", buf);
	}
}
void write_routine(int sock, char *buf)
{
	while(1)
	{
		fgets(buf, BUF_SIZE, stdin); //표준 입력받음
		if(!strcmp(buf,"q\n") || !strcmp(buf,"Q\n")) //q나 Q입력시
		{	
			shutdown(sock, SHUT_WR); //쓰기 종료
			return;
		}
		write(sock, buf, strlen(buf)); //서버로 전송
	}
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}