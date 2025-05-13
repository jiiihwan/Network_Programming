#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
	int sock;  // 소켓 파일 디스크립터
	char buf[BUF_SIZE];  // 데이터를 주고받을 버퍼
	struct sockaddr_in serv_addr;  // 서버 주소 정보를 담을 구조체

	FILE * readfp;  // 읽기용 파일 스트림
	FILE * writefp;  // 쓰기용 파일 스트림
	
	sock=socket(PF_INET, SOCK_STREAM, 0);  // TCP 소켓 생성
	memset(&serv_addr, 0, sizeof(serv_addr));  // 주소 구조체 초기화
	serv_addr.sin_family=AF_INET;  // IPv4 주소체계 설정
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);  // 서버 IP 주소 설정
	serv_addr.sin_port=htons(atoi(argv[2]));  // 서버 포트 번호 설정
  
	connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));  // 서버에 연결
	readfp=fdopen(sock, "r");  // 소켓을 읽기용 파일 스트림으로 변환
	writefp=fdopen(sock, "w");  // 소켓을 쓰기용 파일 스트림으로 변환
  
	while(1)
	{
		if(fgets(buf, sizeof(buf), readfp)==NULL)  // 서버로부터 데이터 수신
			break;
		fputs(buf, stdout);  // 수신한 데이터를 화면에 출력
		fflush(stdout);
	 }  

	fputs("FROM CLIENT: Thank you! \n", writefp);  // 서버에 감사 메시지 전송
	fflush(writefp);
	fclose(writefp); fclose(readfp);  // 파일 스트림 종료
	return 0;
}
