#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;  // 서버 소켓과 클라이언트 소켓 파일 디스크립터
	FILE * readfp;  // 읽기용 파일 스트림
	FILE * writefp;  // 쓰기용 파일 스트림
	
	struct sockaddr_in serv_adr, clnt_adr;  // 서버와 클라이언트 주소 정보를 담을 구조체
	socklen_t clnt_adr_sz;  // 클라이언트 주소 구조체 크기
	char buf[BUF_SIZE]={0,};  // 데이터를 주고받을 버퍼

	serv_sock=socket(PF_INET, SOCK_STREAM, 0);  // TCP 소켓 생성
	memset(&serv_adr, 0, sizeof(serv_adr));  // 주소 구조체 초기화
	serv_adr.sin_family=AF_INET;  // IPv4 주소체계 설정
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);  // 서버 IP 주소 설정 (모든 인터페이스)
	serv_adr.sin_port=htons(atoi(argv[1]));  // 서버 포트 번호 설정
	
	bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr));  // 소켓에 주소 할당
	listen(serv_sock, 5);  // 연결 요청 대기 상태로 진입
	clnt_adr_sz=sizeof(clnt_adr); 
	clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);  // 클라이언트 연결 수락
	
	readfp=fdopen(clnt_sock, "r");  // 소켓을 읽기용 파일 스트림으로 변환
	writefp=fdopen(dup(clnt_sock), "w");  // 소켓을 복제하여 쓰기용 파일 스트림으로 변환
	
	fputs("FROM SERVER: Hi~ client? \n", writefp);  // 클라이언트에게 메시지 전송
	fputs("I love all of the world \n", writefp);
	fputs("You are awesome! \n", writefp);
	fflush(writefp);  // 버퍼의 데이터를 즉시 전송
	
	shutdown(fileno(writefp), SHUT_WR);  // 쓰기 스트림의 출력 스트림을 종료 (half-close)
	fclose(writefp);  // 쓰기 스트림 종료
	
	fgets(buf, sizeof(buf), readfp); fputs(buf, stdout);  // 클라이언트로부터 메시지 수신 및 출력
	fclose(readfp);  // 읽기 스트림 종료
	return 0;
}

/*
swyoon@my_linux:~/tcpip$ gcc sep_serv2.c -o serv2
swyoon@my_linux:~/tcpip$ ./serv2 9190
FROM CLIENT: Thank you! 
*/

