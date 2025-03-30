#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX 1024

int main(int argc, char** argv) {
    int sfd; //UDP에서는 sfd만 존재
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char opCount;
    int opResult;
    int operand[MAX];
    char operator;
    char buf[MAX];

    if(argc < 2) {
        printf("usage:./server localPort\n");
        return 1;
    }

    if((sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error");
        return 1;
    }

    int enable = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); //소켓 재사용할 수 있게 하는 옵션

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    if(bind(sfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind error");
        return 1;
    }       

    while(1){
        len = sizeof(cliaddr);
        //read
        recvfrom(sfd,&buf,MAX,0,(struct sockaddr*)&cliaddr,&len); //buf로 전체 내용(opCount, operand, operator)을 한번에 받기
        
        opCount = buf[0]; //buf값으로 opCount 정의

        if(opCount <= 0) { 
            printf("Server close(%d)\n",opCount);
            close(sfd);
            return 0;
            //break;
        }

        printf("Operand count: %d\n", opCount); //그리고 operand count 출력

        for(int i=0; i<opCount; i++) { //입력한 opCount만큼 반복
            //read(cfd, &operand[i], 4);
            memcpy(&operand[i], &buf[1 + i * 4], 4); //buf에서 oprand값 파싱, 이때 char->int로 받아야하니 memcpy로 복사, operand가 4바이트씩 차지하니까 크기는 4바이트로.
            printf("Operand %d: %d\n", i, operand[i]); //i번째 operand 출력
        }

        opResult = operand[0];  //opResult에 첫번째 숫자 전달

        for(int i=0; i<opCount-1; i++) { 
            operator = buf[1+opCount*4+i]; //operator정의, operand가 4바이트니까 operand개수*4 만큼 띄우는거 주의

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
        }

        printf("Operation result: %d\n", opResult); 
        //write(cfd, &opResult, 4); 
        sendto(sfd, &opResult, 4, 0, (const struct sockaddr*)&cliaddr,len); //클라이언트 소켓에 opResult 전송
    }
    
    close(sfd); //서버 소켓 닫기
    return 0;
}

