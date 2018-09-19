/*************************************************************************
# > File Name: test_thpool.c
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-17 15:09:35
# > Revise Time: 2018-09-19 20:17:06
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include"evbase_threadpool.h"
#include"Threadpool.h"
/* 线程池中的线程，模拟处理业务 */
void *process(void *arg)
{
    printf("thread 0x%x working on task %d\n ",(unsigned int)pthread_self(),*(int *)arg);
    sleep(1);
    printf("task %d is end\n",*(int *)arg);


    return NULL;

}
int main(void)
{
    threadpool_t *thp = threadpool_create(10);
     /* evbase_threadpool_t *ev_thpool = evbase_threadpool_new(1000, 10); */ 
    /* threadpool_t *thp = evbase_threadpool_get_threadpool(ev_thpool) ;*/
    printf("pool inited\r\n");

    int num[20000], i;
    for (i = 0; i < 20; i++) {
        num[i]=i;
        printf("add task %d\n",i);
        threadpool_add(thp, process, (void*)&num[i]);     /* 向线程池中添加任务 */
    }
    while(1);                                          /* 等子线程完成任务 */
    threadpool_destroy(thp);

    return 0;
}

