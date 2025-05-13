#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100        // 메시지 버퍼 크기
#define MAX_CLNT 256        // 최대 클라이언트 수

void * handle_clnt(void * arg);    // 클라이언트 처리를 위한 스레드 함수
void send_msg(char * msg, int len); // 모든 클라이언트에게 메시지 전송
void error_handling(char * msg);    // 에러 처리 함수

int clnt_cnt=0;                     // 현재 접속중인 클라이언트 수
int clnt_socks[MAX_CLNT];          // 클라이언트 소켓 배열
pthread_mutex_t mutx;              // 스레드 동기화를 위한 뮤텍스

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;

	// 포트 번호 인자 확인
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);    // 뮤텍스 초기화
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);  // 서버 소켓 생성

	// 서버 주소 설정
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);  // 모든 IP에서 접속 가능
	serv_adr.sin_port=htons(atoi(argv[1]));      // 포트 번호 설정
	
	// 소켓 바인딩 및 리스닝
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	// 클라이언트 연결 수락 및 처리
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		// 새로운 클라이언트 소켓을 배열에 추가
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++]=clnt_sock;
		pthread_mutex_unlock(&mutx);
	
		// 새로운 클라이언트를 위한 스레드 생성
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);  // 스레드 분리
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);
	return 0;
}
	
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg);
	int str_len=0, i;
	char msg[BUF_SIZE];
	
	// 클라이언트로부터 메시지를 받아 모든 클라이언트에게 전송
	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0)
		send_msg(msg, str_len);
	
	// 클라이언트 연결 종료 시 처리
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // 연결이 끊긴 클라이언트 제거
	{
		if(clnt_sock==clnt_socks[i])    // 현재 처리 중인 클라이언트 소켓을 찾았을 때
		{
			while(i++<clnt_cnt-1)       // 배열의 끝까지 반복
				clnt_socks[i]=clnt_socks[i+1];    // 한 칸씩 앞으로 당겨서 빈 공간 채우기
			break;    // 클라이언트 제거 완료 후 반복문 종료
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}

// 모든 클라이언트에게 메시지 전송
void send_msg(char * msg, int len)   
{
	int i;
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}

// 에러 처리 함수
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}