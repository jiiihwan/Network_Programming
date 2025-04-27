#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/uio.h>  

#define MAX 1024
#define MODE_SIZE 5
#define ID_SIZE 5
void error_handling(char *message); 

int sock; //소켓 선언
//char message[MAX]; //메세지 배열 선언
int str_len;
struct sockaddr_in serv_adr;

int opCount, opResult;

struct iovec vec[3];
char mode[MODE_SIZE];
char id[ID_SIZE];
char buf[MAX];

void vec_init(){
    vec[0].iov_base = mode;
    vec[0].iov_len = MODE_SIZE;
    vec[1].iov_base = id;
    vec[1].iov_len = ID_SIZE;
    vec[2].iov_base = buf;
    vec[2].iov_len = MAX;
}

void caculator_input(){
    printf("Operand count: ");
    scanf("%d", &opCount); //operation Count를 입력받는다
    buf[0] = (char)opCount; //buf배열 첫번째 인덱스에 opCount를 char형태로 저장

    if(buf[0] <= 0) { //opCount가 음수면 소켓 종료
        printf("Overflow will happen(%d)\n", opCount);
        close(sock);
        return ;
    }
    for(int i=0; i<opCount; i++) { //operation Count만큼 반복
        printf("Operand %d: ", i); 
        scanf("%d", (int*)&buf[(i*4)+1]); //숫자를 입력받아 int형태로 4바이트 단위로 저장
    }
    for(int i=0; i<opCount-1; i++) {
        printf("Operator %d: ",i);
        scanf(" %c", &buf[(opCount*4)+1+i]); //operator를 buf배열의 숫자개수x4+1+i 인덱스에 opCount입력받기(buf마지막에 operator들 넣기)
    }
}

int main(int argc, char *argv[])
{
	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0); //소켓 만들기
	if(sock==-1) //에러 발생시
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr)); //서버주소 구조체의 쓰레기값 없애고 초기화
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]); 
	serv_adr.sin_port=htons(atoi(argv[2]));

    vec_init();

    //Mode출력 및 입력받기
    void mode_check();{
        printf("Mode: ");
        scanf("%s", mode);
        if (strcmp(mode, "save") != 0 && strcmp(mode, "load") != 0 && strcmp(mode, "quit") != 0) {
            printf("supported mode: save load quit\n");
            close(sock);
            return 0;
        }
    }

	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1) //서버와 연결
		error_handling("connect() error!");
	else
		puts("Connected....");

    //quit일때 수행
    if (!strcmp(mode, "quit")) {
        puts("quit");;
        writev(sock, vec, 1); //server로 write
        close(sock);
        return 0;
    }

	//ID출력 및 입력받기
    printf("ID: ");
    scanf("%s", id);
    if (strlen(id) != 4) {
        puts("Error: ID length must be 4");
        close(sock);
        return 0;
    }

    //load일 때 수행
    if (!strcmp(mode, "load")) {
        writev(sock, vec, 2); //server로 write
        str_len = read(sock, buf, MAX-1); //server소켓에서 buf에 데이터를 읽어오기
        buf[str_len] = 0;
        printf("%s", buf); //buf 내용 출력
        close(sock);
        return 0;
    }

    //save일 때 수행
    caculator_input();
    //write(sock, buf, 1+(opCount*4)+(opCount-1)); //server소켓에 buf의 데이터를 전달
    writev(sock, vec, 3); //server로 write
    read(sock, &opResult, 4); //server소켓에서 데이터를 읽어와서 opResult에 저장
    printf("Operation result: %d\n", opResult);
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}