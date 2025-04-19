#define _XOPEN_SOURCE 200
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/wait.h>
#include <signal.h>
#define MAX 1024

void read_childproc(int sig);

int main(int argc, char** argv) {
    int sfd, cfd;
    struct sockaddr_in servaddr, cliaddr;
    //socklen_t len;
    char opCount;
    int opResult;
    int operand[MAX];
    char operator;
    char buf[MAX];

    pid_t pid;
	struct sigaction act; 
	socklen_t adr_sz;
	int str_len, state;

    //파이프
    int fds[2];

    if(argc < 2) {
        printf("usage:./server localPort\n");
        return 1;
    }

    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return 1;
    }

    act.sa_handler=read_childproc; //시그널 핸들러 등록
	sigemptyset(&act.sa_mask); //핸들러 초기화 
	act.sa_flags=0; //핸들러 초기화
	state=sigaction(SIGCHLD, &act, 0); //SIGCHLD: 자식 프로세스가 종료될때 부모프로세스에 전달됨, zombie 처리

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));
    
    int enable = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); //reuse

    if(bind(sfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind error");
        return 1;
    }       

    if(listen(sfd, 5) < 0) {
        perror("listen error");
        return 1;
    }

    //파이프 정의
    pipe(fds); // fds[0]: 읽기용, fds[1]: 쓰기용
    pid = fork();
    if (pid == 0) { //자식프로세스
        FILE* fp = fopen("log.txt", "wt"); //파일 생성
        char msgbuf[MAX];
        int i, len;

        while (1){
            len = read(fds[0], msgbuf, MAX);
            fwrite(msgbuf, 1, len, fp);
            fflush(fp);
        }

        fclose(fp);
        return 0;
    }

    while(1){
        adr_sz = sizeof(cliaddr);
        cfd = accept(sfd, (struct sockaddr *)&cliaddr, &adr_sz);

        if(cfd==-1) //실패 시 다음루프로 
			continue;
		else
			puts("new client connected...");

		pid=fork(); //새 클라이언트마다 자식 프로세스를 생성

        if(pid==0) //자식 프로세스는 
		{
			close(sfd); //자식은 필요없는 서버 소켓 닫기

            read(cfd, &opCount, 1); //cfd로부터 최대 1바이트만 읽어서 opCount변수에 저장한다
            if(opCount <= 0) { 
                printf("Save file(%d)\n", opCount);     // 메시지 작성
                //close(fds[1]);
                close(cfd);
                return 0;
            }
    
            //printf("Operand count: %d\n", opCount); //그리고 operand count 출력
    
            for(int i=0; i<opCount; i++) { //입력한 opCount만큼 반복
                read(cfd, &operand[i], 4); //클라이언트로소켓으로부터 4바이트씩 읽고 operand[i]에 저장, read는 이전에 읽은 데이터 다음 위치에서 계속 읽어서 이렇게 가능하다
                //printf("Operand %d: %d\n", i, operand[i]); //i번째 operand 출력
            }
    
            opResult = operand[0];  //opResult에 첫번째 숫자 전달
            
            sprintf(buf, "%d: %d", getpid(), operand[0]);  // 시작: process id + 첫 번째 숫자

            for(int i=0; i<opCount-1; i++) { //operator 빼고 반복
                read(cfd, &operator, 1); //1바이트(char) 읽어서 operator에 저장
                switch (operator) { //operator case에 따른 스위치문
                case '+':
                    opResult += operand[i+1]; //+일경우 opresult변수에 i+1 번째 숫자 덧셈
                    break;
                case '-':
                    opResult -= operand[i+1];
                    break;
                case '*':
                    opResult *= operand[i+1];
                }
                sprintf(buf + strlen(buf), "%c%d", operator, operand[i+1]); //buf에 연산자랑 다음 operand 이어붙이기
            }
    
            write(cfd, &opResult, 4); //클라이언트 소켓에 opResult 전송

            sprintf(buf + strlen(buf), "=%d\n", opResult); //buf에 opResult까지 붙이기
            write(fds[1], buf, strlen(buf)); //파이프로 앞서 만든 자식프로세스에게 계산정보를 process id와 함께 전달
            close(fds[1]);
            printf("%s", buf); 

			close(cfd); //통신 종료 후 클라이언트 소켓 닫기
			return 0;
		}
		else //부모 프로세스는
			close(cfd); //필요없는 클라이언트 소켓 닫기
    }
    
    close(sfd); //서버 소켓 닫기
    return 0;
}

void read_childproc(int sig) //자식 프로세스 종료시 실행되는 시그널 핸들러
{
	pid_t pid;
	int status;
	pid=waitpid(-1, &status, WNOHANG); //좀비 프로세스 제거
	printf("removed proc id: %d \n", pid);
}

