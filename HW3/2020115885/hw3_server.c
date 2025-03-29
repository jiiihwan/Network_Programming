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

    len = sizeof(cliaddr);

    while(1){
        //read(cfd, &opCount, 1); //cfd로부터 최대 1바이트만 읽어서 opCount변수에 저장한다
        recvfrom(cfd,&opCount,1,0,(struct sockaddr*)&cliaddr,&len);
        if(opCount <= 0) { 
            printf("Server close(%d)\n",opCount);
            close(cfd);
            return 0;
            //break;
        }

        printf("Operand count: %d\n", opCount); //그리고 operand count 출력

        for(int i=0; i<opCount; i++) { //입력한 opCount만큼 반복
            //read(cfd, &operand[i], 4); //클라이언트로소켓으로부터 4바이트씩 읽고 operand[i]에 저장, read는 이전에 읽은 데이터 다음 위치에서 계속 읽어서 이렇게 가능하다
            recvfrom(cfd,&operand[i],4,0,(struct sockaddr*)&cliaddr,&len);
            printf("Operand %d: %d\n", i, operand[i]); //i번째 operand 출력
        }

        //read(cfd, &operator, 1); //1바이트(char) 읽어서 operator에 저장
        recvfrom(cfd,&operator,1,0,(struct sockaddr*)&cliaddr,&len);

        opResult = operand[0];  //opResult에 첫번째 숫자 전달
        for(int i=0; i<opCount-1; i++) { //operator 빼고 반복
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
            //read(cfd, &operator, 1); //1바이트(char) 읽어서 operator에 저장
            recvfrom(cfd,&operator,1,0,(struct sockaddr*)&cliaddr,&len);
        }

        printf("Operation result: %d\n", opResult); 
        //write(cfd, &opResult, 4); //클라이언트 소켓에 opResult 전송
        sendto(cfd, &opResult, 4, 0, (const struct sockaddr*)&cliaddr,len);

        close(cfd); //클라이언트 소켓 닫기
    }
    
    close(sfd); //서버 소켓 닫기
    return 0;
}

