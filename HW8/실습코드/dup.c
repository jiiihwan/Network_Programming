#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	// 파일 디스크립터 변수
	int cfd1, cfd2;
	char str1[]="Hi~ \n";
	char str2[]="It's nice day~ \n";

	// 표준 출력 복제
	cfd1=dup(1); //표준 출력(1번 파일 디스크립터)을 복제
	cfd2=dup2(cfd1, 7); //복제된 파일 디스크립터를 7번으로 지정
	
	printf("fd1=%d, fd2=%d \n", cfd1, cfd2);
	
	close(1); //표준 출력을 닫음
	printf("printf test\n"); //표준 출력을 닫았으므로 출력되지 않음
	write(1, str1, sizeof(str1)); //닫힌 1번 파일 디스크립터에 쓰기 시도
	write(cfd1, str1, sizeof(str1)); //복제된 파일 디스크립터를 통해 "Hi~" 출력

	close(cfd1); //cfd1 닫기
	write(cfd1, str1, sizeof(str1)); //닫힌 파일 디스크립터에 쓰기 시도 -> Hi 출력 안됨
	write(cfd2, str2, sizeof(str2)); //cfd2를 통해 "It's nice day~" 출력
	close(cfd2); //cfd2 닫기
	return 0;
}
