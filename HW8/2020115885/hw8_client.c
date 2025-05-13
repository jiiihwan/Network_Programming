#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 100
#define NAME_SIZE 20
	
void * send_msg(void * arg);    // 메시지 전송 스레드 함수
void * recv_msg(void * arg);    // 메시지 수신 스레드 함수
void error_handling(char * msg); // 에러 처리 함수
	
char name[NAME_SIZE]="[DEFAULT]"; // 사용자 이름
char msg[BUF_SIZE];              // 메시지 버퍼
	
int main(int argc, char *argv[])
{
	int sock;                    // 소켓 파일 디스크립터
	struct sockaddr_in serv_addr; // 서버 주소 구조체
	pthread_t snd_thread, rcv_thread; // 송신, 수신 스레드
	void * thread_return;        // 스레드 반환값

	// 프로그램 실행 시 필요한 인자 검사
	if(argc!=4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	}
	
	// 사용자 이름 설정
	sprintf(name, "[%s]", argv[3]);
	// TCP 소켓 생성
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	// 서버 주소 구조체 초기화 및 설정
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	// 서버에 연결
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
	// 송신 스레드와 수신 스레드 생성
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	
	// 스레드 종료 대기
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	
	close(sock);  // 소켓 닫기
	return 0;
}
	
// 메시지 전송 스레드 함수
void * send_msg(void * arg)   
{
	int sock=*((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	while(1) 
	{
		fgets(msg, BUF_SIZE, stdin);  // 표준 입력에서 메시지 읽기
		// 'q' 또는 'Q' 입력 시 프로그램 종료
		if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n")) 
		{
			close(sock);
			exit(0);
		}
		// 이름과 메시지를 합쳐서 전송
		sprintf(name_msg,"%s %s", name, msg);
		write(sock, name_msg, strlen(name_msg));
	}
	return NULL;
}
	
// 메시지 수신 스레드 함수
void * recv_msg(void * arg)   
{
	int sock=*((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	while(1)
	{
		// 서버로부터 메시지 수신
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		if(str_len==-1) {
			return (void*)-1;
		}

		name_msg[str_len]=0;  // 문자열 끝에 NULL 추가
		fputs(name_msg, stdout);  // 수신된 메시지 출력
	}
	return NULL;
}
	
// 에러 처리 함수
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
