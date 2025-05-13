#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#define BUF_SIZE 3  // 버퍼 크기를 3바이트로 설정

int main(int argc, char *argv[])
{
    int fd, len, dfd;  // fd: 원본 파일 디스크립터, dfd: 복제된 파일 디스크립터
    char buf[BUF_SIZE];

    // test.txt 파일을 읽기 전용으로 열기
    fd=open("test.txt", O_RDONLY);
    // fd를 복제하여 새로운 파일 디스크립터 dfd 생성
    dfd = dup(fd);

    // 원본 파일 디스크립터(fd)로 파일 읽기
    len=read(fd, buf, sizeof(buf));
    printf("fd: %s\n", buf);
    // 원본 파일 디스크립터 닫기
    close(fd);

    // 복제된 파일 디스크립터(dfd)로 파일 읽기
    len = read(dfd, buf, sizeof(buf));
    printf("dfd: %s\n", buf);
    // 복제된 파일 디스크립터 닫기
    close(dfd);

    return 0;
}