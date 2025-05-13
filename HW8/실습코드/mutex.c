#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#define NUM_THREAD	2  // 생성할 스레드의 개수

// 스레드 함수 선언
void * thread_inc(void * arg);  // 증가 스레드 함수
void * thread_des(void * arg);  // 감소 스레드 함수

long long num=0;  // 공유 변수
pthread_mutex_t mutex;  // 뮤텍스 객체 선언

int main(int argc, char *argv[]) 
{
	pthread_t thread_id[NUM_THREAD];  // 스레드 ID 배열
	int i;
	
	pthread_mutex_init(&mutex, NULL);  // 뮤텍스 초기화

	// 스레드 생성
	for(i=0; i<NUM_THREAD; i++)
	{
		if(i%2)
			pthread_create(&(thread_id[i]), NULL, thread_inc, NULL);  // 홀수 인덱스: 증가 스레드
		else
			pthread_create(&(thread_id[i]), NULL, thread_des, NULL);  // 짝수 인덱스: 감소 스레드
	}	

	// 모든 스레드의 종료 대기
	for(i=0; i<NUM_THREAD; i++)
		pthread_join(thread_id[i], NULL);

	printf("result: %lld \n", num);  // 최종 결과 출력
	pthread_mutex_destroy(&mutex);  // 뮤텍스 제거
	return 0;
}

// 증가 스레드 함수
void * thread_inc(void * arg) 
{
	int i;
	pthread_mutex_lock(&mutex);  // 뮤텍스 잠금
	for(i=0; i<50000; i++)
		num+=1;  // 공유 변수 증가
	pthread_mutex_unlock(&mutex);  // 뮤텍스 해제
	return NULL;
}

// 감소 스레드 함수
void * thread_des(void * arg)
{
	int i;
	for(i=0; i<50000; i++)
	{
		pthread_mutex_lock(&mutex);  // 뮤텍스 잠금
		printf("%lld\n", num);  // 현재 값 출력
		num-=1;  // 공유 변수 감소
		pthread_mutex_unlock(&mutex);  // 뮤텍스 해제
	}
	return NULL;
}

/*
컴파일 및 실행 방법:
gcc mutex.c -D_REENTRANT -o mutex -lpthread
./mutex

실행 결과:
result: 0
*/
