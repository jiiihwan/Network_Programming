#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/uio.h>

#define BUF_SIZE 1024
#define NAME_SIZE 5

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);

char name[NAME_SIZE];

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void *thread_return;

    if (argc != 4) {
        printf("Usage: %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }

    if (strlen(argv[3]) != 4) {
        printf("ID have to be 4\n");
        exit(1);
    }

    sprintf(name, "%s", argv[3]);

    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
    close(sock);
    return 0;
}

void *send_msg(void *arg) {
    int sock = *((int*)arg);
    char req[BUF_SIZE];
    struct iovec vec[2];
    
    vec[0].iov_base = name;
    vec[0].iov_len = NAME_SIZE;
    vec[1].iov_base = req;
    vec[1].iov_len = BUF_SIZE;

    while (1) {
        //printf("> ");
        char input[BUF_SIZE];
        fgets(input, BUF_SIZE, stdin);
        input[strcspn(input, "\n")] = 0;  // 개행 제거

        // 파싱 준비
        char *token = strtok(input, " ");
        if (!token) continue;

        int temp = atoi(token);
        char opCount = (char)temp;
        if (opCount <= 0) {  // char로 변환 후 0 이하면 오버플로우
            printf("Overflow value(%d) - client closed\n", opCount);
            close(sock);
            exit(1);
        }

        req[0] = opCount;

        //operand 저장
        for (int i = 0; i < opCount; i++) {
            token = strtok(NULL, " ");
            if (!token) break;
            int val = atoi(token);
            memcpy(&req[1 + i * 4], &val, 4);  // 4바이트 int 저장
        }

        //operator 저장
        for (int i = 0; i < opCount - 1; i++) {
            token = strtok(NULL, " ");
            if (!token) break;
            req[1 + opCount * 4 + i] = token[0];  // 1바이트 char 저장
        }

        writev(sock, vec, 2);  // 이름 + 계산 데이터 전송
    }

    return NULL;
}

void *recv_msg(void *arg) 
{
    int sock=*((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	while(1)
	{
		// 서버로부터 메시지 수신
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		if(str_len==-1) {
			return (void*)-1;
		}
    
		name_msg[str_len]=0;  // 문자열 끝에 NULL 추가
		fputs(name_msg, stdout);  // 수신된 메시지 출력
	}
	return NULL;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
