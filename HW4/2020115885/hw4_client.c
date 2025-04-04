#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 1024
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int i;
	struct hostent *host; 
	struct sockaddr_in addr; //주소로 host를 구하니 선언

    int sock_type; //소켓 타입 옵션 버퍼
	socklen_t optlen; //옵션 길이 전달용
    
	int sd; //소켓 디스크립터
	FILE *fp; //파일 포인터
	char buf[BUF_SIZE];
	int read_cnt; //읽은 바이트 수
	struct sockaddr_in serv_adr; //서버 주소 정보 구조체 선언
    

    if(argc==2){
        host=gethostbyname(argv[1]); //인자로 도메인을 받은걸 그대로 host로 쓴다
        if(!host)
		error_handling("gethostbyname... error");

        printf("gethostbyname()\n");
        printf("Official name: %s \n", host->h_name);
        
        for(i=0; host->h_aliases[i]; i++)
            printf("Aliases %d: %s \n", i+1, host->h_aliases[i]);
        
        printf("Address type: %s \n", 
            (host->h_addrtype==AF_INET)?"AF_INET":"AF_INET6");

        char *ip_address; //ip주소 담을 문자열 선언 
        for(i=0; host->h_addr_list[i]; i++){
            ip_address = inet_ntoa(*(struct in_addr*)host->h_addr_list[i]);
            printf("IP addr %d: %s \n", i+1, ip_address);
        }
        ip_address = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]); //첫번째 주소값을 ip_address에 저장

        memset(&addr, 0, sizeof(addr));
        addr.sin_addr.s_addr=inet_addr(ip_address); //ip 주소 값을 인자로 받기
    
        host=gethostbyaddr((char*)&addr.sin_addr, 4, AF_INET); //(조회할 IP주소, IPv4는 4바이트, IPv4), 반환값은 hostent*
        if(!host)
            error_handling("\ngethostbyaddr... error\n");
    
        printf("\ngethostbyaddr()\n");
        printf("Official name: %s \n", host->h_name);  //name출력
    
        for(i=0; host->h_aliases[i]; i++) //별칭들 출력
            printf("Aliases %d: %s \n", i+1, host->h_aliases[i]);
        
        printf("Address type: %s \n",  //adress type출력
            (host->h_addrtype==AF_INET)?"AF_INET":"AF_INET6");
    
        for(i=0; host->h_addr_list[i]; i++){ //adress list 출력
            printf("IP addr %d: %s \n", i+1, inet_ntoa(*(struct in_addr*)host->h_addr_list[i])); 
            //원래 in_addr 구조체안의 주소는 사람이 못읽는 16진수로 되어있어서 바꿔줘야함
        }   
        return 0;
    }

	else if(argc==3) {  //인자가 3개인 경우
        sd=socket(PF_INET, SOCK_STREAM, 0); //TCP

        memset(&serv_adr, 0, sizeof(serv_adr)); //서버 주소 정보 구조체 초기화
        serv_adr.sin_family=AF_INET;
        serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
        serv_adr.sin_port=htons(atoi(argv[2]));

        optlen=sizeof(sock_type);
        getsockopt(sd, SOL_SOCKET, SO_TYPE, (void*)&sock_type, &optlen);
        printf("This socket type is : %d(%d) \n", sock_type, SOCK_STREAM);

        fp=fopen("copy.txt", "wb+"); //copy.txt 파일을 binary읽기,쓰기 전용으로 읽는다, *fopen()->포인터로 쓴다. =>열고자 하는 파일의 경로

        connect(sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr)); //서버에 연결 요청
        
        while((read_cnt=read(sd, buf, BUF_SIZE ))!=0) //읽은 바이트수가 버퍼의 사이즈랑 같지 않을때까지
            fwrite((void*)buf, 1, read_cnt, fp); //fwrite(파일에 쓸 데이터가 저장된 메모리 주소, 데이터 바이트, 개수, 파일 포인터), copy.txt에 버퍼의 내용을 쓴다.
        
        puts("Received file data"); //puts는 지동 줄바꿈이 있는 문자열 출력
        while ((read_cnt = fread(buf, 1, BUF_SIZE, fp)) > 0) //버퍼에 든 문자열 개수를 반환해서 마지막 조각에 도달할때까지
            write(sd, buf, read_cnt);//write(파일디스크립터, 출력할 데이터의 포인터 또는 문자열, 바이트 수), 버퍼에 찬 문자열만큼 보내기
        fclose(fp); //열었던 파일 스트림 종료, 버퍼 비우고 리소스 해제 
        close(sd); //소켓 닫기
        return 0;
    }
}

void error_handling(char *message)
{
	fputs(message, stderr); //에러일때 message출력
	fputc('\n', stderr);  //fputc는 문자 한개 출력
	exit(1); //exit1은 에러종료
}