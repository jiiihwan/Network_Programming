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
	int serv_sd, clnt_sd; //서버와 클라이언트 소켓 정리
	FILE * fp; //파일 포인터
	char buf[BUF_SIZE];
	int read_cnt;
	
	struct sockaddr_in serv_adr, clnt_adr; //구조체 선언
	socklen_t clnt_adr_sz;
	
	if(argc!=2) { //인자가 2개 아니면 사용법 출력
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}
	
	fp=fopen("test.txt", "rb"); 
	serv_sd=socket(PF_INET, SOCK_STREAM, 0); //TCP

	memset(&serv_adr, 0, sizeof(serv_adr)); //서버 주소 정보 구조체 초기화
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	bind(serv_sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr)); //서버 주소 바인딩
	listen(serv_sd, 5); //대기 큐 5개
	
	clnt_adr_sz=sizeof(clnt_adr); //클라이언트 주소 사이즈
	clnt_sd=accept(serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz); //클라이언트의 연결요청을 수락하고 클라이언트 소켓 생성
	
	while(1)
	{
		read_cnt=fread((void*)buf, 1, BUF_SIZE, fp); //버퍼에 든 문자열 개수를 반환
		if(read_cnt<BUF_SIZE) //버퍼 사이즈보다 작을때(파일 끝네 도달했을때)
		{
			write(clnt_sd, buf, read_cnt); //마지막 조각을 전달
			break; 
		}
		write(clnt_sd, buf, BUF_SIZE); //버퍼 사이즈만큼 전달
	}
	
	shutdown(clnt_sd, SHUT_WR);	//서버가 클라이언트에게 더이상 write할거 없다고 알림, read는 할 수 있음
	read(clnt_sd, buf, BUF_SIZE); //클라이언트로부터 read
	printf("Message from client: %s \n", buf); //버퍼에 든 메세지 출력
	
	fclose(fp); //파일포인터 삭제?
	close(clnt_sd); close(serv_sd); //서버와 클라이언트 소켓 닫기
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
