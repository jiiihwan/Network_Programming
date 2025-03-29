#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX 1024

int main(int argc, char** argv) {
    int sockfd;
    struct sockaddr_in serv_addr, cli_addr; 
    socklen_t len;
    char buf[MAX];

    if(argc < 2) {
        printf("usage: ./server PORT");
        return -1;
    }

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(sockfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind error");
        return -1;
    }    

    while(1) {
        len = sizeof(cli_addr); //클라이언트 주소의 길이
        memset(buf, 0, sizeof(buf)); //버퍼 초기화
        int recv_len = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&cli_addr, &len); //메시지 수신
        //printf("%s %d\n", buf, recv_len);
        if(buf[0] == 'q' || buf[0] == 'Q') { 
            break;
        }
        strcat(buf, " by server\n"); //문자열 연결, buf 뒤에 by server를 붙힌다.
        sendto(sockfd, buf, strlen(buf), 0, (const struct sockaddr*)&cli_addr, len); //붙힌 buf를 전송
    }

    close(sockfd);
    return 0;
}