#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sd; //소켓 디스크립터
	FILE *fp; //파일 포인터
	char buf[BUF_SIZE];
	int read_cnt; //읽은 바이트 수
	struct sockaddr_in serv_adr; //서버 주소 정보 구조체 선언
	
	if(argc!=3) {  //인자가 3개가 아니면 사용법 출력
		printf("Usage: %s <IP> <port>\n", argv[0]); 
		exit(1);
	}
	
	fp=fopen("copy.txt", "wb"); //copy.txt 파일을 binary쓰기 전용으로 읽는다, *fopen()->포인터로 쓴다. =>열고자 하는 파일의 경로
	sd=socket(PF_INET, SOCK_STREAM, 0); //TCP

	memset(&serv_adr, 0, sizeof(serv_adr)); //서버 주소 정보 구조체 초기화
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));

	connect(sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr)); //서버와 연결
	
	while((read_cnt=read(sd, buf, BUF_SIZE ))!=0) //읽은 바이트수가 버퍼의 사이즈랑 같지 않을때까지 
		fwrite((void*)buf, 1, read_cnt, fp); //fwrite(파일에 쓸 데이터가 저장된 메모리 주소, 데이터 바이트, 개수, 파일 포인터), copy.txt.에 쓴다
	
	puts("Received file data"); //puts는 지동 줄바꿈이 있는 문자열 출력
	write(sd, "Thank you", 10); //write(파일디스크립터, 출력할 데이터의 포인터 또는 문자열, 바이트 수), 서버에 문자열을 보낸다
	//참고로 위에꺼가 10바이트인 이유는 뒤에 NULL까지 보내기 때문
	fclose(fp); //열었던 파일 스트림 종료, 버퍼 비우고 리소스 해제 
	close(sd); //소켓 닫기
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr); //에러일때 message출력
	fputc('\n', stderr);  //fputc는 문자 한개 출력
	exit(1); //exit1은 에러종료
}