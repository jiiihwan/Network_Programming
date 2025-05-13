#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// 스레드 메인 함수 선언
void* thread_main(void *arg);

int main(int argc, char *argv[]) 
{
	pthread_t t_id;  // 스레드 ID를 저장할 변수
	int thread_param=5;  // 스레드에 전달할 매개변수
	void * thr_ret;  // 스레드의 반환값을 저장할 포인터
	
	// 새로운 스레드 생성
	// t_id: 스레드 ID를 저장할 변수의 주소
	// NULL: 기본 스레드 속성 사용
	// thread_main: 스레드가 실행할 함수
	// &thread_param: 스레드 함수에 전달할 매개변수
	if(pthread_create(&t_id, NULL, thread_main, (void*)&thread_param)!=0)
	{
		puts("pthread_create() error");
		return -1;
	}; 	

	// 스레드의 종료를 기다림
	// t_id: 종료를 기다릴 스레드의 ID
	// &thr_ret: 스레드의 반환값을 저장할 포인터의 주소
	if(pthread_join(t_id, &thr_ret)!=0)
	{
		puts("pthread_join() error");
		return -1;
	};

	// 스레드가 반환한 메시지 출력
	printf("Thread return message: %s \n", (char*)thr_ret);
	free(thr_ret);  // 동적 할당된 메모리 해제
	return 0;
}

// 스레드가 실행할 함수
// arg: 스레드 생성 시 전달된 매개변수
void* thread_main(void *arg) 
{
	int i;
	int cnt=*((int*)arg);  // 매개변수를 정수형으로 변환
	char * msg=(char *)malloc(sizeof(char)*50);  // 메시지를 저장할 메모리 동적 할당
	strcpy(msg, "Hello, I'am thread~ \n");  // 메시지 초기화

	for(i=0; i<cnt; i++)
	{
		sleep(1);  // 1초 대기
		puts("running thread");	 // 스레드 실행 메시지 출력
	}
	return (void*)msg;  // 동적 할당된 메시지 반환
}