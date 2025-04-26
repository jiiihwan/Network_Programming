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
#include <sys/uio.h>  

#define MOD_SIZE 100
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

    char opCount;
    int opResult;
    int operand[BUF_SIZE];
    char operator[BUF_SIZE]; //원래 단일 문자변수인데 여기선 배열로 선언함

	socklen_t adr_sz; //주소 사이즈 구조체
	int fd_max, str_len, fd_num, i; //가장 큰 fd번호, 읽은데이터길이
	char buf[BUF_SIZE];

	pid_t pid;
	int state;
	struct sigaction act;
	int fdsA[2], fdsB[2];

    struct iovec vec[3];
    char mode[MOD_SIZE]; // mode ("save", "load", "quit")
    char id[ID_SIZE];   // id (길이 4)
    char result[BUF_SIZE]; // 계산 데이터

    // 각 버퍼 설정
    vec[0].iov_base = mode;
    vec[0].iov_len = sizeof(mode);
    vec[1].iov_base = id;
    vec[1].iov_len = sizeof(id);
    vec[2].iov_base = result;
    vec[2].iov_len = sizeof(result);

    typedef struct data
    {
        char id[ID_SIZE];
        char req[REQ_SIZE];
    }data;

    data dataArray[DATA_MAX];
    int cnt = 0;


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
        int cnt = 0;
		while(1)
		{
            read(fdsA[0], &dataArray[cnt], sizeof(dataArray[cnt]));
            cnt++;



		}
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
					str_len=readv(i, vec, 3); //vec정보 read
					if(str_len==0)    // close request!
					{
						FD_CLR(i, &reads); //i번째 fd에서 fd정보 삭제
						close(i);
						printf("closed client: %d \n", i);
					}
					else
					{
                        if (strcmp(mode, "save") == 0){
                            //read(i, &opCount, 1); //cfd로부터 최대 1바이트만 읽어서 opCount변수에 저장한다
                            opCount = result[0];  // opCount 저장

                            printf("Operand count: %d\n", opCount); //그리고 operand count 출력
                    
                            for(int j=0; j<opCount; j++) { //입력한 opCount만큼 반복
                                //read(i, &operand[j], 4); //클라이언트로소켓으로부터 4바이트씩 읽고 operand[i]에 저장, read는 이전에 읽은 데이터 다음 위치에서 계속 읽어서 이렇게 가능하다
                                memcpy(&operand[j], &result[1 + j*4], 4);  // 4바이트씩 operand 복원
                                printf("Operand %d: %d\n", j, operand[j]); //j번째 operand 출력
                            }
                    
                            opResult = operand[0];  //opResult에 첫번째 숫자 전달
                            
                            sprintf(buf , "%d", operand[0]);  // 시작:  첫 번째 숫자
                
                            for(int j=0; j<opCount-1; j++) { //operator 빼고 반복
                                //read(clnt_sock, &operator, 1); //1바이트(char) 읽어서 operator에 저장
                                operator[j] = result[1 + opCount*4 + j];  // 그 뒤에는 operator들
                                switch (operator[j]) { //operator case에 따른 스위치문
                                case '+':
                                    opResult += operand[j+1]; //+일경우 opresult변수에 i+1 번째 숫자 덧셈
                                    break;
                                case '-':
                                    opResult -= operand[j+1];
                                    break;
                                case '*':
                                    opResult *= operand[j+1];
                                }
                                sprintf(buf + strlen(buf), "%c%d", operator[j], operand[j+1]); //buf에 연산자랑 다음 operand 이어붙이기
                            }
                    
                            write(i, &opResult, 4); //클라이언트 소켓에 opResult 전송
                
                            sprintf(buf + strlen(buf), "=%d\n", opResult); //buf에 opResult까지 붙이기

                            strcpy(dataArray[cnt].id, id);    // id 저장
                            strcpy(dataArray[cnt].req, buf);  // 계산 수식 저장
                            write(fdsA[1], &dataArray[cnt], sizeof(dataArray[cnt]));
                            cnt++;
                            printf("save to %s\n", id);
                        }
                        else if (strcmp(mode, "load") == 0) {
                            printf("load from %s\n", id);
                            write(fdsA[1], mode, MOD_SIZE);
                            write(fdsA[1], id, ID_SIZE);




                        }
                        
					
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