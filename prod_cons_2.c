#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m)\
    do\ 
    {\
	perror(m);\
	exit(EXIT_FAILURE);\
    }while(0)

#define CONSUMERS_COUNT 1
#define PRODUCERS_COUNT 1
#define BUFFSIZE 10

int g_buffer[BUFFSIZE];

unsigned short in = 0;
unsigned short out = 0;
unsigned short produce_id = 0;
unsigned short consume_id = 0;

//声明生产者关注的空格子和消费者关注的满格子
sem_t g_sem_full;
sem_t g_sem_empty;
//声明互斥量
pthread_mutex_t g_mutex;

//什么一个线程数组
pthread_t g_thread[CONSUMERS_COUNT + PRODUCERS_COUNT];


//消费者函数
void* consume(void *arg)
{
    //将传参拿出来并且释放原空间
    int i;
    int num = *(int*)arg;
    free(arg);

    while(1)
    {
	printf("%d wait buffer not empty\n", num);
	//等待信号量，信号量的值会减1
	sem_wait(&g_sem_empty);
	//对互斥量进行加锁
	pthread_mutex_lock(&g_mutex);

	for(i = 0; i < BUFFSIZE; i++){
	    printf("%d", i);
	    if(g_buffer[i] == -1)
		printf("%s", "null");
	    else
		printf("%d", g_buffer[i]);

	    if(i == out)
		printf("\t<--consume");

	    printf("\n");
	}
	
	consume_id  = g_buffer[out];
	printf("%d begin consume product %d\n", num, consume_id);
	g_buffer[out] = -1;
	out = (out + 1) % BUFFSIZE;
	printf("%d end consume product %d\n", num, consume_id);
	//完成操作，解锁互斥量
	pthread_mutex_unlock(&g_mutex);
	//消费完成之后，对空格子进行+1操作
	sem_post(&g_sem_full);
	sleep(1);
    }
    return NULL;
}


//生产者函数
void* produce(void *arg)
{
    int i;
    //拿到传参并释放传参的空间
    int num = *(int*)arg;
    free(arg);
    while(1)
    {
	printf("%d wait buffer not full\n", num);
	//信号量等待，信号量-1
	sem_wait(&g_sem_full);
	pthread_mutex_lock(&g_mutex);
	//轮询生产过程
	for(i = 0; i < BUFFSIZE; i++)
	{
	    printf("%d", i);
	    if(g_buffer[i] == -1)
		printf("%s ", "null");
	    else 
		printf("%d", g_buffer[i]);

	    if(i == in)
		printf("\t<--produce");

	    printf("\n");
	}
	
	printf("%d begin produce product %d\n", num, produce_id);
	g_buffer[in] = produce_id;
	in = (in + 1) % BUFFSIZE;
	printf("%d end produce product %d\n", num, produce_id++);
	pthread_mutex_unlock(&g_mutex);
	sem_post(&g_sem_empty);
	sleep(5);
    }
    return NULL;
}



int main(void)
{
    int i;
    for(i = 0; i < BUFFSIZE; i++)
	g_buffer[i] = -1;

    //初始化信号量
    sem_init(&g_sem_full, 0, BUFFSIZE);
    sem_init(&g_sem_empty, 0, 0);
    
    //初始化互斥量
    pthread_mutex_init(&g_mutex, NULL);
    
    for(i = 0; i < CONSUMERS_COUNT; i++)
    {
	int *p = (int*)malloc(sizeof(int));
	*p = i;
	//创建消费者线程
	pthread_create(&g_thread[i], NULL, consume, (void*)p);
    }

    for(i = 0; i < PRODUCERS_COUNT; i++)
    {
	int *p = (int*)malloc(sizeof(int));
	*p = i;
	//创建生产者线程
	pthread_create(&g_thread[CONSUMERS_COUNT + i], NULL, produce, \
	    (void*)p);
    }
    //线程等待，回收资源
    for(i = 0; i < CONSUMERS_COUNT + PRODUCERS_COUNT; i++)
	pthread_join(g_thread[i], NULL);

    //销毁信号量和互斥量
    sem_destroy(&g_sem_full);
    sem_destroy(&g_sem_empty);
    pthread_mutex_destroy(&g_mutex);

    return 0;
    
}







