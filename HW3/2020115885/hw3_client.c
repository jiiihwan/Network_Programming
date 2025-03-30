#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX 1024

int main(int argc, char** argv) {
    int sfd;
    char buf[MAX];
    struct sockaddr_in servaddr;
    int opCount, opResult;
    socklen_t len; //주소 길이 전달용 변수추가

    if(argc < 3) {
        printf("usage: ./client address port");
        return -1;
    }

    if((sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socekt error");
        return -1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    printf("Operand count: ");
    scanf("%d", &opCount); //operation Count를 입력받는다
    buf[0] = (char)opCount; //buf배열 첫번째 인덱스에 opCount를 char형태로 저장

    len = sizeof(servaddr);

    if(buf[0] <= 0) { //opCount가 음수면 소켓에 buf내용을 보내고 서버소켓 종료
        //write(sfd, buf, 1);
        sendto(sfd, buf, 1, 0, (struct sockaddr*)&servaddr, len);
        close(sfd);
        return 0;
    }

    for(int i=0; i<opCount; i++) { //operation Count만큼 반복
        printf("Operand %d: ", i); 
        scanf("%d", (int*)&buf[(i*4)+1]); //숫자를 입력받아 int형태로 4바이트 단위로 저장
    }

    for(int i=0; i<opCount-1; i++) {
        printf("Operator %d: ",i);
        scanf(" %c", &buf[(opCount*4)+1+i]); //operator를 buf배열의 숫자개수x4+1+i 인덱스에 opCount입력받기(buf마지막에 operator들 넣기)
    }

    //write
    sendto(sfd, buf, 1+(opCount*4)+(opCount-1), 0, (struct sockaddr*)&servaddr, len); //server소켓에 buf의 데이터를 한번에 전달

    //read(sfd, &opResult, 4); 
    recvfrom(sfd, &opResult, 4, 0, (struct sockaddr*)&servaddr, &len); //server소켓에서 데이터를 읽어와서 opResult에 저장
    printf("Operation result: %d\n", opResult);

    close(sfd);
    return 0;
}