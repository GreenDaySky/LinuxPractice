#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

#define CONSUMERS_COUNT 2
#define PRODUCERS_COUNT 2

//声明链表结构
struct msg{
    struct msg *next;
    int num;
};

struct msg *head = NULL;


//声明条件变量和互斥量
pthread_cond_t cond;
pthread_mutex_t mutex;
pthread_t threads[CONSUMERS_COUNT + PRODUCERS_COUNT];

void *consumer(void *p)
{
    //将传参的内容拿出来并且释放原先的空间
    int num = *(int*)p;
    free(p);
    struct msg *mp;
    for(;;){
	//保持唯一访问性互斥量加锁
	pthread_mutex_lock(&mutex);
	//如果没有内容可以消费，条件变量等待条件满足
	while(head == NULL){
	    printf("%d begin wait a condition...\n", num);
	    pthread_cond_wait(&cond, &mutex);
	}
	//开始消费过程
	printf("%d end wait a condition...\n", num);
	printf("%d begin consume produce...\n", num);
	mp = head;
	head = mp->next;
	//解锁并将消费过的内容存储空间进行释放
	pthread_mutex_unlock(&mutex);
	printf("Consume %d\n", mp->num);
	free(mp);
	printf("%d end consume product...\n", num);
	sleep(rand() % 5);
    }
}

void *producer(void *p)
{
    //声明链表结构体
    struct msg *mp;
    //把数拿出来之后将原先传参的空间释放掉
    int num = *(int*)p;
    free(p);
    for(;;){
	 printf("%d begin produce product...\n", num);
	 //为声明的结构体开辟空间并随机赋值
	 mp = (struct msg*)malloc(sizeof(struct msg));
	 mp->num = rand() % 1000 + 1;
	 printf("produce %d\n", mp->num);
	 //对其进行互斥量加锁
	 pthread_mutex_lock(&mutex);
	 //把刚刚生产的内容放在链表的表头
	 mp->next = head;
	 head = mp;
	 printf("%d end produce product...\n", num);
	//条件变量唤醒等待
	 pthread_cond_signal(&cond);
	 //解锁
	 pthread_mutex_unlock(&mutex);
	 sleep(rand() % 5);
    }
}
    
int main(void)
{
    srand(time(NULL));
   
   //创建互斥量和条件变量
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mutex, NULL);

    
    //生产过程
    int i;
    for(i = 0; i < CONSUMERS_COUNT; i++){
	int *p = (int*)malloc(sizeof(int));
	*p = i;
	pthread_create(&threads[i], NULL, consumer, (void*)p);
    }

    //消费过程
    for(i = 0; i < PRODUCERS_COUNT; i++){
	int *p = (int*)malloc(sizeof(int));
	*p = i;
	pthread_create(&threads[CONSUMERS_COUNT + i], NULL, producer, (void*)p);
    }

    //线程等待
    for(i = 0; i < CONSUMERS_COUNT + PRODUCERS_COUNT; i++)
	pthread_join(threads[i], NULL);

    //销毁互斥量和条件变量
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
    return 0;
}




















