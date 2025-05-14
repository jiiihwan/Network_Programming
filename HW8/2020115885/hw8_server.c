#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/uio.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define NAME_SIZE 5
#define MAX_CLNT 256

void *handle_clnt(void *arg);
void send_all(char *msg, int len);
void error_handling(char *msg);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;


int main(int argc, char *argv[]) 
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t adr_sz;
    pthread_t t_id;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    pthread_mutex_init(&mutx, NULL);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    while (1) {
        adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutx);

        printf("Connected client port: %d\n", ntohs(clnt_adr.sin_port));

        pthread_create(&t_id, NULL, handle_clnt, (void *)&clnt_sock);
        //int *clnt_sock_ptr = malloc(sizeof(int));   // 새 소켓 변수 힙에 동적 할당
        //*clnt_sock_ptr = clnt_sock;
        //pthread_create(&t_id, NULL, handle_clnt, (void *)clnt_sock_ptr);
        pthread_detach(t_id);
    
    }

    //close(serv_sock);
    //return 0;
}

void *handle_clnt(void *arg) 
{
    int clnt_sock = *((int *)arg);  // 값 복사
    //free(arg);                      // 동적 할당된 메모리 해제
    int str_len=0;
    struct iovec vec[2];
    char name[NAME_SIZE];
    char req[BUF_SIZE];
    char msg[BUF_SIZE*2];

    int opCount;
    int operand[BUF_SIZE];
    char operator[BUF_SIZE];
    int opResult;
    char buf[BUF_SIZE];

    vec[0].iov_base = name;
    vec[0].iov_len = NAME_SIZE;
    vec[1].iov_base = req;
    vec[1].iov_len = BUF_SIZE;

    // 클라이언트로부터 메시지를 받아 계산하고 모든 클라이언트에게 전송
    while (1) {
        int str_len = readv(clnt_sock, vec, 2);
        if (str_len <= 0) {
            // 클라이언트가 정상 종료하거나 강제종료(예: exit, Ctrl+C 등)
            break;
        }

        opCount = req[0];

        for(int j=0; j<opCount; j++) 
        { //입력한 opCount만큼 반복
            memcpy(&operand[j], &req[1 + j*4], 4);  // 4바이트씩 operand 복원
        }

        opResult = operand[0];  //opResult에 첫번째 숫자 전달
        sprintf(buf, "%d", operand[0]);  // 시작:  첫 번째 숫자

        for(int j=0; j<opCount-1; j++) 
        { //operator 빼고 반복
            operator[j] = req[1 + opCount*4 + j];  // 그 뒤에는 operator들
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

        snprintf(msg, sizeof(msg), "[%s] %s=%d\n", name, buf, opResult);
        send_all(msg, strlen(msg));
    }

    // 클라이언트 연결 종료 시 처리
    int i;
    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++) 
    {
        if (clnt_sock == clnt_socks[i]) 
        {
            while (i++ < clnt_cnt-1)
                clnt_socks[i] = clnt_socks[i+1];
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    printf("client close\n");
    return NULL;
}

void send_all(char *msg, int len) 
{
    int i;  
    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++)
        write(clnt_socks[i], msg, len);
    pthread_mutex_unlock(&mutx);
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}