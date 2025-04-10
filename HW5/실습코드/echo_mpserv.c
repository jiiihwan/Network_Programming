#define _XOPEN_SOURCE 200
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *message);
void read_childproc(int sig);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	
	pid_t pid;
	struct sigaction act; 
	socklen_t adr_sz;
	int str_len, state;
	char buf[BUF_SIZE];
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	act.sa_handler=read_childproc; //시그널 핸들러 등록
	sigemptyset(&act.sa_mask); //핸들러 초기화 
	act.sa_flags=0; //핸들러 초기화
	state=sigaction(SIGCHLD, &act, 0); //SIGCHLD: 자식 프로세스가 종료될때 부모프로세스에 전달됨, zombie 처리

	serv_sock=socket(PF_INET, SOCK_STREAM, 0); 
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	while(1)
	{
		adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
		if(clnt_sock==-1) //실패 시 다음루프로 
			continue;
		else
			puts("new client connected...");
		pid=fork(); //새 클라이언트마다 자식 프로세스를 생성
		if(pid==-1) //fork 실패시 클라이언트 소켓 닫기
		{
			close(clnt_sock); 
			continue;
		}
		if(pid==0) //자식 프로세스는 
		{
			close(serv_sock); //자식은 필요없는 서버 소켓 닫기
			while((str_len=read(clnt_sock, buf, BUF_SIZE))!=0) //에코기능, 클라이언트에서 받은 데이터를 그대로 다시 전송
				write(clnt_sock, buf, str_len);
			
			close(clnt_sock); //통신 종료 후 클라이언트 소켓 닫기
			puts("client disconnected...");
			return 0;
		}
		else //부모 프로세스는
			close(clnt_sock); //필요없는 클라이언트 소켓 닫기
	}
	close(serv_sock); //서버 종료시 소켓 닫기
	return 0;
}

void read_childproc(int sig) //자식 프로세스 종료시 실행되는 시그널 핸들러
{
	pid_t pid;
	int status;
	pid=waitpid(-1, &status, WNOHANG); //좀비 프로세스 제거
	printf("removed proc id: %d \n", pid);
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}