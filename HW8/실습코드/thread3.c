#include <stdio.h>
#include <pthread.h>

// 스레드 합계 계산 함수 선언
void * thread_summation(void * arg); 

int sum=0;  // 전역 변수로 선언된 합계 (스레드들이 공유)

int main(int argc, char *argv[])
{
	pthread_t id_t1, id_t2;  // 두 개의 스레드 ID
	int range1[]={1, 5};     // 첫 번째 스레드의 계산 범위 (1~5)
	int range2[]={6, 10};    // 두 번째 스레드의 계산 범위 (6~10)
	
	// 첫 번째 스레드 생성 (1~5의 합계 계산)
	pthread_create(&id_t1, NULL, thread_summation, (void *)range1);
	// 두 번째 스레드 생성 (6~10의 합계 계산)
	pthread_create(&id_t2, NULL, thread_summation, (void *)range2);

	// 두 스레드의 종료를 기다림
	pthread_join(id_t1, NULL);
	pthread_join(id_t2, NULL);
	printf("result: %d \n", sum);  // 최종 합계 출력
	return 0;
}

// 스레드가 실행할 합계 계산 함수
// arg: 계산할 범위를 담은 배열의 포인터
void * thread_summation(void * arg) 
{
	int start=((int*)arg)[0];  // 계산 시작 값
	int end=((int*)arg)[1];    // 계산 종료 값

	while(start<=end)
	{
		sum+=start;  // 전역 변수 sum에 현재 값을 더함
		start++;
	}
	return NULL;  // 스레드 종료
}
