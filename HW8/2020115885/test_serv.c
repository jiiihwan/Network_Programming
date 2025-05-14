#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/uio.h>

#define MAX_CLNT 100
#define BUF_SIZE 1024
#define NAME_SIZE 4

int clnt_socks[MAX_CLNT];
int clnt_count = 0;
pthread_mutex_t mutx;

void *handle_clnt(void *arg);
int calculate(char *expr, char *formatted_expr);
void error_handling(const char *msg);

void print_client_sockets_nolock() {
    printf("현재 연결된 클라이언트 소켓 목록 (%d개):\n", clnt_count);
    for (int i = 0; i < clnt_count; i++) {
        printf("clnt_socks[%d] = %d\n", i, clnt_socks[i]);
    }
}



int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    pthread_t t_id;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    pthread_mutex_init(&mutx, NULL);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
    listen(serv_sock, 5);

    while (1) {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
    
        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_count++] = clnt_sock;
        pthread_mutex_unlock(&mutx);
        // 연결된 소켓 출력
        print_client_sockets_nolock();  // 여기에 출력
    
        printf("Connected client port: %d\n", ntohs(clnt_adr.sin_port));
    
        int *sock_ptr = malloc(sizeof(int));  
        *sock_ptr = clnt_sock;
    
        pthread_create(&t_id, NULL, handle_clnt, (void*)sock_ptr);
        pthread_detach(t_id);
    }
}

void *handle_clnt(void *arg) {
    int clnt_sock = *((int*)arg);
    free(arg);
    char id[NAME_SIZE + 1];
    char expr[BUF_SIZE];
    struct iovec vec[2];

    vec[0].iov_base = id;
    vec[0].iov_len = NAME_SIZE;
    vec[1].iov_base = expr;
    vec[1].iov_len = BUF_SIZE;

    while (1) {
        int str_len = readv(clnt_sock, vec, 2);
        if (str_len <= 0)
            break;

        id[NAME_SIZE] = '\0';
        expr[BUF_SIZE - 1] = '\0';
        char expr_copy[BUF_SIZE];
        strncpy(expr_copy, expr, BUF_SIZE);
       
        char formatted_expr[BUF_SIZE];
        int result = calculate(expr_copy, formatted_expr);
        char result_str[BUF_SIZE + NAME_SIZE + 10];

        if (result == -99999) {
            char overflow_msg[64];
            int count_val = atoi(expr);
            snprintf(overflow_msg, sizeof(overflow_msg),"Overflow value(%d) - client closed\n", (char)count_val);
            write(clnt_sock, overflow_msg, strlen(overflow_msg));
            goto cleanup;
        }
    
        snprintf(result_str, sizeof(result_str), "[%s] %s=%d\n", id, formatted_expr, result);

        pthread_mutex_lock(&mutx);
        for (int i = 0; i < clnt_count; i++) {
            write(clnt_socks[i], result_str, strlen(result_str));
        }
        pthread_mutex_unlock(&mutx);
    }
    cleanup:
        pthread_mutex_lock(&mutx);
        for (int i = 0;i<clnt_count;i++) {
            if(clnt_socks[i] == clnt_sock) {
                for(int j = i; j < clnt_count - 1; j++) {
                    clnt_socks[j] = clnt_socks[j + 1];
                }
                clnt_count--;
                break;
            }
        }
        print_client_sockets_nolock();  // 여기에 출력
        pthread_mutex_unlock(&mutx);

    close(clnt_sock);
    printf("client close\n");
    return NULL;
}

int calculate(char *expr, char *formatted_expr) {
    int operands[255];
    char operators[255];
    int operand_count = 0;

    char *token = strtok(expr, " ");
    if (token == NULL) return 0;

    operand_count = atoi(token);
    if(operand_count == 0 || operand_count > 127)
        return -99999;

    for (int i = 0; i < operand_count; i++) {
        token = strtok(NULL, " ");
        if (token == NULL) return 0;
        operands[i] = atoi(token);
    }

    for (int i = 0; i < operand_count - 1; i++) {
        token = strtok(NULL, " ");
        if (token == NULL) return 0;
        operators[i] = token[0];
    }

    int result = operands[0];
    for (int i = 1; i < operand_count; i++) {
        switch (operators[i - 1]) {
            case '+': result += operands[i]; break;
            case '-': result -= operands[i]; break;
            case '*': result *= operands[i]; break;
            case '/': if (operands[i] != 0) result /= operands[i]; break;
        }
    }
    
    sprintf(formatted_expr, "%d", operands[0]);
    for (int i = 1; i < operand_count; i++) {
        char buf[32];
        sprintf(buf, "%c%d", operators[i - 1], operands[i]);
        strcat(formatted_expr, buf);
    }

    return result;
}


void error_handling(const char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
