#define _XOPEN_SOURCE 200
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/wait.h>

#define BUF_SIZE 100
#define ID_SIZE 100
#define REQ_SIZE 100
#define DATA_MAX 100
void error_handling(char *buf);
void read_childproc(int sig);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock; //서버와 클라이언트 소켓 선언
	struct sockaddr_in serv_adr, clnt_adr;
	struct timeval timeout; //select 대기시간용
	fd_set reads, cpy_reads; //select용 fd_set형 집합, 원본과 복사본

	socklen_t adr_sz; //주소 사이즈 구조체
	int fd_max, str_len, fd_num, i; //가장 큰 fd번호, 읽은데이터길이
	char buf[BUF_SIZE];

	pid_t pid;
	int state;
	struct sigaction act;
	int fdsA[2], fdsB[2];


    typedef struct data
    {
        char id[ID_SIZE];
        char req[REQ_SIZE];
    }data;
    data dataArray[DATA_MAX];
    


	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	act.sa_handler=read_childproc; //좀비프로세스 처리하는 핸들러 정의
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	state=sigaction(SIGCHLD, &act, 0); //child가 죽었을때 핸들러 실행


	serv_sock=socket(PF_INET, SOCK_STREAM, 0); //소켓 만들기
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");

	pipe(fdsA); //파이프a 생성
	pipe(fdsB); //파이프b 생성
	pid=fork();
	if(pid==0) //자식프로세스는
	{
		FILE * fp=fopen("log.txt", "a+"); //로그파일 열고
		char msgbuf[BUF_SIZE], modebuf[BUF_SIZE];
		int i, len;
		memset(msgbuf, 0, sizeof(msgbuf)); //버퍼 초기화
		while(1)
		{
			len=read(fdsA[0], msgbuf, BUF_SIZE); 
			// 4bytes : mode read
			fwrite((void*)msgbuf, 1, len, fp);
			fflush(fp);
		}
		fclose(fp);
		return 0;
	}

	FD_ZERO(&reads); //reads(fd집합) 초기화
	FD_SET(serv_sock, &reads); //서버소켓을 read에 추가 - 클라이언트 접속 요청을 받아야하니까
	fd_max=serv_sock; //fd_max 초기값 세팅

	while(1)
	{
		cpy_reads=reads; //원본으로부터 복사
		timeout.tv_sec=5; //대기시간 정의
		timeout.tv_usec=0;

		if((fd_num=select(fd_max+1, &cpy_reads, 0, 0, &timeout))==-1) //select로 등록된 소켓들 감시 
			break;
		
		if(fd_num==0) //이벤트 없으면 타임아웃 
			continue;

		for(i=0; i<fd_max+1; i++)
		{
			if(FD_ISSET(i, &cpy_reads)) //i로 전달된 파일디스크립터 정보가 있을경우 
			{
				if(i==serv_sock)     // connection request!
				{
					adr_sz=sizeof(clnt_adr);
					clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
					FD_SET(clnt_sock, &reads); //클라이언트소켓의 fd정보 등록
					if(fd_max<clnt_sock)
						fd_max=clnt_sock; //fd_max 값 설정
					printf("connected client: %d \n", clnt_sock);
				}
				else    // read message!, 클라이언트로부터 데이터 수신
				{
					str_len=read(i, buf, BUF_SIZE); //readv로 바꿔줘야함!!!!!!!!!!!!!!
					if(str_len==0)    // close request!
					{
						FD_CLR(i, &reads); //i번째 fd에서 fd정보 삭제
						close(i);
						printf("closed client: %d \n", i);
					}
					else
					{
						write(i, buf, str_len);    // echo!
						write(fdsA[1], buf, str_len); //파이프로 자식프로세스에 기록 요청
					}
				}
			}
		}
	}
	close(serv_sock);
	return 0;
}

void read_childproc(int sig) {
  pid_t pid;
  int status;
  pid = waitpid(-1, &status, WNOHANG);
  printf("removed proc id: %d \n", pid);
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}