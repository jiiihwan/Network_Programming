#include <stdio.h>
#include <pthread.h>

// 스레드 메인 함수 선언
void* thread_main(void *arg);

int main(int argc, char *argv[]) 
{
	pthread_t t_id;  // 스레드 ID를 저장할 변수
	int thread_param=5;  // 스레드에 전달할 매개변수
	
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
	
	sleep(10);  // 메인 스레드 10초 대기
	puts("end of main");
	return 0;
}

// 스레드가 실행할 함수
// arg: 스레드 생성 시 전달된 매개변수
void* thread_main(void *arg) 
{
	int i;
	int cnt=*((int*)arg);  // 매개변수를 정수형으로 변환
	for(i=0; i<cnt; i++)
	{
		sleep(1);  // 1초 대기
		puts("running thread");	 // 스레드 실행 메시지 출력
	}
	return NULL;  // 스레드 종료
}