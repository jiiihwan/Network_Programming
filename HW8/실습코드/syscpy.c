#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#define BUF_SIZE 3  // 버퍼 크기를 3바이트로 설정

int main(int argc, char *argv[])
{
    int fd1, fd2, len;  // fd1: 소스 파일 디스크립터, fd2: 대상 파일 디스크립터
    char buf[BUF_SIZE]; // 파일 복사에 사용할 버퍼
    struct timeval start, stop;  // 시간 측정을 위한 구조체

    // 소스 파일(news.txt)을 읽기 전용으로 열기
    fd1=open("news.txt", O_RDONLY);
    // 대상 파일(cpy.txt)을 쓰기 전용으로 열기
    // O_CREAT: 파일이 없으면 생성
    // O_TRUNC: 파일이 있으면 내용을 지우고 새로 시작
    fd2=open("cpy.txt", O_WRONLY|O_CREAT|O_TRUNC);

    // 복사 시작 시간 측정
    gettimeofday(&start, NULL);
    
    // 버퍼 크기만큼 반복해서 파일 복사
    while((len=read(fd1, buf, sizeof(buf)))>0)
        write(fd2, buf, len);
    
    // 복사 종료 시간 측정
    gettimeofday(&stop, NULL);
    
    // 복사에 걸린 시간을 밀리초 단위로 출력
    // tv_sec: 초 단위, tv_usec: 마이크로초 단위
    printf("%lu ms\n", (stop.tv_sec - start.tv_sec) * 1000 + 
           (stop.tv_usec - start.tv_usec) / 1000);

    // 파일 디스크립터 닫기
    close(fd1);
    close(fd2);
    return 0;
}