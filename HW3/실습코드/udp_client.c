#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX 1024

int main(int argc, char** argv) {
    int sockfd;
    struct sockaddr_in serv_addr;
    socklen_t len;
    char buf[MAX];

    if(argc < 3) { 
        printf("usage: ./client IP PORT");
        return -1;
    }

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { //socket type이 stream에서 DGRAM으로 바뀜
        perror("socket error");
        return -1;
    }

    serv_addr.sin_family = AF_INET; //IPv4
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); //문자열 IP -> 정수 IP
    serv_addr.sin_port = htons(atoi(argv[2])); //포트를 네트워크 바이트 순서로 변환

    len = sizeof(serv_addr);    
    while(1) {
        printf("Input Message (Q or q) : ");
        scanf("%s", buf);
        //printf("%s\n", buf);
        //sendto로 메세지 보낸다.
        int res = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); //TCP에 비해 끝에 두개 추가
        if(res < 0) { //sendto 는 전송한 바이트 수를 반환하기 때문에 음수면 실패한 것.
            perror("send error");
            break;
        }

        if(buf[0] == 'q' || buf[0] == 'Q') {
            break;
        }

        memset(buf, 0, sizeof(buf)); //버퍼를 0으로 채움
        res = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&serv_addr, &len);
        if(res < 0) {
            perror("recvfrom error");
            break;
        }
        
        printf("%s", buf);
    }
 
    close(sockfd);
    return 0;
}