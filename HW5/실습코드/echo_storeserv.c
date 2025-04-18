#define _XOPEN_SOURCE 200
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 100

void error_handling(char* message);
void read_childproc(int sig);

char buf[BUF_SIZE];
int main(int argc, char* argv[]) {
  int serv_sock, clnt_sock;
  struct sockaddr_in serv_adr, clnt_adr;
  int fds[2];

  pid_t pid;
  struct sigaction act;
  socklen_t adr_sz;
  int str_len, state;
  
  if (argc != 2) {
    printf("Usage : %s <port>\n", argv[0]);
    exit(1);
  }

  act.sa_handler = read_childproc;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  state = sigaction(SIGCHLD, &act, 0);

  serv_sock = socket(PF_INET, SOCK_STREAM, 0);
  memset(&serv_adr, 0, sizeof(serv_adr));
  serv_adr.sin_family = AF_INET;
  serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_adr.sin_port = htons(atoi(argv[1]));

  if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
    error_handling("bind() error");
  if (listen(serv_sock, 5) == -1)
    error_handling("listen() error");

  pipe(fds); 
  pid = fork();
  if (pid == 0) { //자식프로세스
    FILE* fp = fopen("echomsg.txt", "wt"); //파일 생성
    char msgbuf[BUF_SIZE];
    int i, len;

    for (i = 0; i < 10; i++) { //최대 10번까지 읽기
      len = read(fds[0], msgbuf, BUF_SIZE); //pipe에서 읽기
      fwrite((void*)msgbuf, 1, len, fp); //파일로 저장
    }
    fclose(fp);
    return 0;
  }

  while (1) {
    adr_sz = sizeof(clnt_adr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
    if (clnt_sock == -1) //오류 시
      continue;
    else
      puts("new client connected...");

    pid = fork();
    if (pid == 0) { //자식프로세스
      close(serv_sock); //필요없는 서버 소켓은 닫기
      while ((str_len = read(clnt_sock, buf, BUF_SIZE)) != 0) { //클라이언트로부터 read
        write(clnt_sock, buf, str_len); //클라이언트 소켓에 echo
        write(fds[1], buf, str_len); //파일 저장을 위해 파이프로 보내기
      }

      close(clnt_sock); //클라이언트 소켓 닫기
      puts("client disconnected...");
      return 0;

    } else //부모 프로세스
      close(clnt_sock); //필요없는 클라이언트 소켓은 닫기
  }
  close(serv_sock); //서버 소켓 닫기
  return 0;
}

void read_childproc(int sig) {
  pid_t pid;
  int status;
  pid = waitpid(-1, &status, WNOHANG);
  printf("removed proc id: %d \n", pid);
}
void error_handling(char* message) {
  fputs(buf, stderr);
  fputc('\n', stderr);
  exit(1);
}