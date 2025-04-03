#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int i;
	struct hostent *host; 
	struct sockaddr_in addr; //주소로 host를 구하니 선언

	if(argc!=2) { 
		printf("Usage : %s <IP>\n", argv[0]);
		exit(1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr=inet_addr(argv[1]);

	host=gethostbyaddr((char*)&addr.sin_addr, 4, AF_INET); //(조회할 IP주소, IPv4는 4바이트, IPv4), 반환값은 hostent*
	if(!host)
		error_handling("gethost... error");

	printf("Official name: %s \n", host->h_name);  //name출력

	for(i=0; host->h_aliases[i]; i++) //별칭들 출력
		printf("Aliases %d: %s \n", i+1, host->h_aliases[i]);
	
	printf("Address type: %s \n",  //adress type출력
		(host->h_addrtype==AF_INET)?"AF_INET":"AF_INET6");

	for(i=0; host->h_addr_list[i]; i++) //adress list 출력
		printf("IP addr %d: %s \n", i+1,
			inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));	//원래 in_addr 구조체안의 주소는 사람이 못읽는 16진수로 되어있어서 바꿔줘야함
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}