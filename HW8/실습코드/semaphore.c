#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

// 함수 선언
void * read(void * arg);
void * accu(void * arg);

// 전역 세마포어 변수 선언
static sem_t sem_one;
static sem_t sem_two;
static int num;

int main(int argc, char *argv[])
{
	pthread_t id_t1, id_t2;
	// 세마포어 초기화
	sem_init(&sem_one, 0, 0);
	sem_init(&sem_two, 0, 1);

	// 스레드 생성
	pthread_create(&id_t1, NULL, read, NULL);
	pthread_create(&id_t2, NULL, accu, NULL);

	// 스레드 종료 대기
	pthread_join(id_t1, NULL);
	pthread_join(id_t2, NULL);

	// 세마포어 소멸
	sem_destroy(&sem_one);
	sem_destroy(&sem_two);
	return 0;
}

// 숫자를 입력받는 스레드 함수
void * read(void * arg)
{
	int i;
	for(i=0; i<5; i++)
	{
		fputs("Input num: ", stdout);

		// sem_two가 0이 될 때까지 대기
		sem_wait(&sem_two);
		scanf("%d", &num);
		// sem_one을 증가시켜 accu 스레드가 진행할 수 있도록 함
		sem_post(&sem_one);
	}
	return NULL;	
}

// 입력받은 숫자들의 합을 계산하는 스레드 함수
void * accu(void * arg)
{
	int sum=0, i;
	for(i=0; i<5; i++)
	{
		// sem_one이 0이 될 때까지 대기
		sem_wait(&sem_one);
		sum+=num;
		// sem_two를 증가시켜 read 스레드가 진행할 수 있도록 함
		sem_post(&sem_two);
	}
	printf("Result: %d \n", sum);
	return NULL;
}