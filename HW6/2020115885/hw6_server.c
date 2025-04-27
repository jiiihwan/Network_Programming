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

#define MOD_SIZE 5
#define ID_SIZE 5
#define BUF_SIZE 1024
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

    //계산기 변수들
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

    //mode와 id
    struct iovec vec[3];
    char mode[MOD_SIZE]; // mode ("save", "load", "quit")
    char id[ID_SIZE];   // id (길이 4)
    char result[BUF_SIZE]; // 계산 데이터

    // 각 버퍼 설정
    vec[0].iov_base = mode;
    vec[0].iov_len = MOD_SIZE;
    vec[1].iov_base = id;
    vec[1].iov_len = ID_SIZE;
    vec[2].iov_base = result;
    vec[2].iov_len = BUF_SIZE;

    //save와 load에 사용할 id와 result 담을 구조체
    typedef struct data
    {
        char id[ID_SIZE];
        char req[REQ_SIZE];
    }data;

    data dataArray[DATA_MAX];

    int cnt = 0;
    char load_buf[BUF_SIZE];

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
	
    int enable = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); //reuse

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
            read(fdsA[0], mode, MOD_SIZE); //부모프로세스로부터 mode값 식별

            if(strcmp(mode, "save") == 0 ){ 
                read(fdsA[0], &dataArray[cnt], sizeof(dataArray[cnt])); // id와 결과값 dataArray구조체로 받아서 저장
                cnt++;
            }
            else if(strcmp(mode, "load") == 0 ){
                read(fdsA[0], id, ID_SIZE); //부모프로세스로부터 id값 식별
                memset(result, 0 ,sizeof(result)); //result배열 초기화
                for (int i = 0; i < cnt; i++){  //dayaArray 개수만큼 순환
                    if(strcmp(id, dataArray[i].id) == 0 ){ //받아온 id와 dataArray에 있는id가 일치하면
                        sprintf(load_buf, "%s: %s", dataArray[i].id, dataArray[i].req); //load_buf에 id: result 꼴로 저장
                        strcat(result, load_buf); //result에 이어붙히기
                    }
                }
                if (result[0]){ //result에 문자가 존재하면
                    write(fdsB[1], result, strlen(result));
                }
                else{ //result에 아무것도 없으면
                    char not_exist[BUF_SIZE] = "Not exist\n";
                    write(fdsB[1], not_exist, strlen(not_exist));
                }

            }




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
                            write(fdsA[1], mode, MOD_SIZE); //mode정보 자식프로세스로 보내기
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
                            
                            printf("Operation result: %d\n", opResult);
                            write(i, &opResult, 4); //클라이언트 소켓에 opResult 전송
                
                            sprintf(buf + strlen(buf), "=%d\n", opResult); //buf에 opResult까지 붙이기

                            strcpy(dataArray[cnt].id, id);    // id 저장
                            strcpy(dataArray[cnt].req, buf);  // 계산 수식 저장
                            write(fdsA[1], &dataArray[cnt], sizeof(dataArray[cnt]));
                            cnt++;
                            printf("save to %s\n", id);

                            close(i);
                            FD_CLR(i, &reads);
                            printf("closed client: %d \n", i);
                        }
                        else if (strcmp(mode, "load") == 0) {
                            printf("load from %s\n", id);
                            write(fdsA[1], mode, MOD_SIZE); //mode정보 자식프로세스로 보내기
                            write(fdsA[1], id, ID_SIZE); //id정보 자식프로세스로 보내기

                            str_len = read(fdsB[0], load_buf, sizeof(load_buf)); //자식프로세스로부터 result 문자열 읽어서 load_buf에 저장

                            write(i, load_buf, str_len); //현재 파일디스크립터에 result 문자열 보내기
                            
                            close(i);
                            FD_CLR(i, &reads);
                            printf("closed client: %d \n", i);
                        }
                        else if (strcmp(mode, "quit") == 0) {
                            write(fdsA[1], mode, MOD_SIZE); //mode정보 자식프로세스로 보내기
                            puts("quit");

                            close(i);
                            FD_CLR(i, &reads);
                            printf("closed client: %d \n", i);
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