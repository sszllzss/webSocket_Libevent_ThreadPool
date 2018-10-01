/*************************************************************************
# > File Name: Threadpool.cpp
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-16 18:16:10
# > Revise Time: 2018-10-01 15:23:50
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<signal.h>
#include"include/debug.h"
#include"include/Threadpool.h"
#include"include/config.h"
struct threadpool_task_t{
    void *(*function)(void *);
    void *arg;
};
struct threadpool_t{
    pthread_mutex_t lock; //用于锁住本结构体
    pthread_mutex_t thread_counter;//记录忙线程格式锁 bufsy_thr_num
    pthread_cond_t queue_not_full;//当任务队列满时，添加任务的线程阻塞，等候该条件变量
    pthread_cond_t queue_not_empty;//任务队列不玩空时，通知等候任务的线程

    pthread_t adjust_tid;//线程管理者tip
    pthread_list_t *threads;//存放线程池中每个线程的tid，列表
    thread_pool_task_queue_t *threadpool_task_queue;//任务队列
    unsigned int min_thr_num;//线程池最小线程数
    unsigned int busy_thr_num;//忙碌线程个数
    unsigned int wait_exit_thr_num;//要销毁的线程个数

    int shutdown;//标志位，线程池使用状态， true 或 false
};
#ifndef DEFAULT_TIME
#define DEFAULT_TIME 500                 /*100ms检测一次*/
#endif
#ifndef MIN_WAIT_TASK_NUM
#define MIN_WAIT_TASK_NUM 10            /*如果queue_size > MIN_WAIT_TASK_NUM 添加新的线程到线程池*/ 
#endif
#ifndef DEFAULT_THREAD_VARY 
#define DEFAULT_THREAD_VARY 10          /*每次创建和销毁线程的个数*/
#endif
#ifndef TH_CREATE_RETRY_NUM
#define TH_CREATE_RETRY_NUM 2           /* 线程创建失败重试次数 */
#endif
void *threadpool_thread(void *threadpool);//线程函数
void *adjust_thread(void *threadpool);//线程管理者线程函数
int threadpool_destroy( threadpool_t *poll);//关闭线程池
/*******************************************************************************
 * 名称: is_thread_alive
 * 功能: 判断线程是否存活
 * 形参: *tip:线程ID
 * 返回: 线程tip存活还回true 或者还回 false
 * 说明: 无
 ******************************************************************************/
int is_thread_alive(pthread_t tid)
{
    int kill_rc = pthread_kill(tid, 0);     //发0号信号，测试线程是否存活
    if (kill_rc == ESRCH) {
        return false;
    }

    return true;
}
/*******************************************************************************
 * 名称: threadpool_create
 * 功能: 创建线程池
 * 形参: *min_thr_num:线程最小线程数 
 * 返回: 创建成果还回 线程池类型指针 threadpool_t 失败 还回NULL 
 * 说明: 无
 ******************************************************************************/
threadpool_t *threadpool_create(int min_thr_num)
{
    threadpool_t *pool=NULL;
    int i;
    do{
        /*  开辟threadpool结构内存空间   */
        if((pool = (threadpool_t *) malloc(sizeof(threadpool_t)))==NULL)
        {
            De_perror("malloci thrread fail");
            break;
        }
        /* 清空线程池结构体 */
        memset(pool, 0, sizeof(threadpool_t));

        pool->min_thr_num = min_thr_num;//线程最小值
        pool->shutdown = false;//不关闭线程
        /* 创建线程列表 */
        pool->threads = new pthread_list_t();
        if(pool->threads == NULL)
        {
            De_fprintf(stderr,"new pthread_list_t fail\r\n");
            break;
        }

        /* 创建任务队列 */
        pool->threadpool_task_queue = new thread_pool_task_queue_t();
        if(pool->threadpool_task_queue == NULL)
        {
            De_fprintf(stderr,"new threadpool_task_queue_t fail\r\n");
            break;
        }
        /* 初始化互斥琐、条件变量 */
        if (pthread_mutex_init(&(pool->lock), NULL) != 0
            || pthread_mutex_init(&(pool->thread_counter), NULL) != 0
            || pthread_cond_init(&(pool->queue_not_empty), NULL) != 0
            || pthread_cond_init(&(pool->queue_not_full), NULL) != 0)
        {
            De_fprintf(stderr,"init the lock or cond fail");
            break;
        }
        /* 启动 min_thr_num 个 work thread */
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//设置线程分离
        for (i = 0; i < min_thr_num; i++) {
            pthread_t pid=0;
            int ret = pthread_create(&pid, &attr, threadpool_thread, (void *)pool);/*pool指向当前线程池*/
            if(ret !=0)
            {
                De_fprintf(stderr, "creatd thread fail:%s\r\n", strerror(ret));
                break;
            }
            pool->threads->push_back(pid); 
            /* De_printf("start thread 0x%x...\n", (unsigned int)pid); */
        }
        pthread_attr_destroy(&attr);
        if(i!=min_thr_num)
            break;
        pthread_create(&(pool->adjust_tid), NULL, adjust_thread, (void *)pool);/* 启动管理者线程 */
        return pool;
    }while(0);
    threadpool_destroy(pool);

    return NULL;
}
/*******************************************************************************
 * 名称: threadpool_add
 * 功能: 向线程池中 添加一个任务
 * 形参: *pool:线程池结构指针
 *       *function:任务函数
 *       *arg:任务函数的参数
 * 返回: 永远还回0
 * 说明: 无
 ******************************************************************************/
int threadpool_add(threadpool_t *pool, void*(*function)(void *arg), void *arg)
{
    pthread_mutex_lock(&(pool->lock));

    /* ==为真，队列已经满， 调wait阻塞 */
    while ((pool->threadpool_task_queue->max_size() == pool->threadpool_task_queue->size()) && (!pool->shutdown)) {
        pthread_cond_wait(&(pool->queue_not_full), &(pool->lock));

    }
    if (pool->shutdown) {
        pthread_mutex_unlock(&(pool->lock));

    }


    /*添加任务到任务队列里*/
    threadpool_task_t task;
    task.function = function;
    task.arg = arg;
    pool->threadpool_task_queue->push_back(task);
    /*添加完任务后，队列不为空，唤醒线程池中 等待处理任务的线程*/
    pthread_cond_signal(&(pool->queue_not_empty));
    pthread_mutex_unlock(&(pool->lock));

    return 0;

}
/*******************************************************************************
 * 名称: threadpool_thread
 * 功能: 线程池中各个工作线程 
 * 形参: *threadpool:线程池结构指针 
 * 返回: NULL
 * 说明: 无
 ******************************************************************************/
/**/
void *threadpool_thread(void *threadpool)
{
    threadpool_t *pool = (threadpool_t *)threadpool;
    threadpool_task_t task;

    while (true) {
        /*刚创建出线程，等待任务队列里有任务，否则阻塞等待任务队列里有任务后再唤醒接收任务*/
        pthread_mutex_lock(&(pool->lock));

        /*queue_size == 0 说明没有任务，调 wait 阻塞在条件变量上, 若有任务，跳过该while*/
        while ((pool->threadpool_task_queue->size() == 0) && (!pool->shutdown)) {  
            /* De_printf("thread 0x%x is waiting\n", (unsigned int)pthread_self()); */
            pthread_cond_wait(&(pool->queue_not_empty), &(pool->lock));

            /*清除指定数目的空闲线程，如果要结束的线程个数大于0，结束线程*/
            if (pool->wait_exit_thr_num > 0) {
                pool->wait_exit_thr_num--;

                /*如果线程池里线程个数大于最小值时可以结束当前线程*/
                if ((unsigned)pool->threads->size() > pool->min_thr_num) {
                    /* De_printf("thread 0x%x is exiting\n", (unsigned int)pthread_self()); */
                    pool->threads->remove(pthread_self());
                    pthread_mutex_unlock(&(pool->lock));
                    pthread_exit(NULL);

                }

            }
        }

        /*如果指定了true，要关闭线程池里的每个线程，自行退出处理*/
        if (pool->shutdown) {
            pool->threads->remove(pthread_self());
            pthread_mutex_unlock(&(pool->lock));
            /* De_printf("thread 0x%x is exiting\n", (unsigned int)pthread_self()); */
            pthread_exit(NULL);     /* 线程自行结束 */

        }

        /*从任务队列里获取任务, 是一个出队操作*/
        thread_pool_task_queue_t *task_queue; 
        task_queue = pool->threadpool_task_queue;
        task.function = (*task_queue->begin()).function;
        task.arg = (*task_queue->begin()).arg;
        task_queue->pop_front();

        /*通知可以有新的任务添加进来*/
        pthread_cond_broadcast(&(pool->queue_not_full));

        /*任务取出后，立即将 线程池琐 释放*/
        pthread_mutex_unlock(&(pool->lock));

        /*执行任务*/ 
        /* De_printf("thread 0x%x start working\n", (unsigned int)pthread_self()); */
        pthread_mutex_lock(&(pool->thread_counter));                            /*忙状态线程数变量琐*/
        pool->busy_thr_num++;                                                   /*忙状态线程数+1*/
        pthread_mutex_unlock(&(pool->thread_counter));
        (*(task.function))(task.arg);                                           /*执行回调函数任务*/
        //task.function(task.arg);                                              /*执行回调函数任务*/
        //
        //        /*任务结束处理*/ 
        /* De_printf("thread 0x%x end working\n", (unsigned int)pthread_self()); */
        pthread_mutex_lock(&(pool->thread_counter));
        pool->busy_thr_num--;                                       /*处理掉一个任务，忙状态数线程数-1*/
        pthread_mutex_unlock(&(pool->thread_counter));

    }

    pthread_exit(NULL);

}

/*******************************************************************************
 * 名称: adjust_thread
 * 功能: 线程管理者线程
 * 形参: *pool:线程池结构指针
 * 返回: NULL
 * 说明: 无
 ******************************************************************************/
void *adjust_thread(void *threadpool)
{
    int i;
    threadpool_t *pool = (threadpool_t *)threadpool;
    while (!pool->shutdown) {

        usleep(DEFAULT_TIME*1000);                                    /*定时 对线程池管理*/

        pthread_mutex_lock(&(pool->lock));
        unsigned int queue_size = pool->threadpool_task_queue->size();                      /* 关注 任务数 */
        unsigned int live_thr_num = pool->threads->size();                  /* 存活 线程数 */
        unsigned int max_size_th = pool->threads->max_size();
        pthread_mutex_unlock(&(pool->lock));

        pthread_mutex_lock(&(pool->thread_counter));
        unsigned int busy_thr_num = pool->busy_thr_num;                  /* 忙着的线程数 */
        pthread_mutex_unlock(&(pool->thread_counter));
        /* 创建新线程 算法： 任务数大于最小线程池个数, 且存活的线程数少于最大线程个数时 如：30>=10 && 40<100*/
        if (queue_size >= MIN_WAIT_TASK_NUM && live_thr_num < max_size_th) {

            pthread_mutex_lock(&(pool->lock));  
            int add = 0;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//设置线程分离
            /*一次增加 DEFAULT_THREAD 个线程*/
            for(i=0;i<DEFAULT_THREAD_VARY
                &&pool->threads->size() < pool->threads->max_size();i++)
            {
                pthread_t pid = 0;
                int ret = pthread_create( &pid, &attr,threadpool_thread, (void*)pool );     
                if(ret != 0)
                {
                    if(add<=TH_CREATE_RETRY_NUM)
                    {
                        add++;
                        i--;//i可以减为 -1 -1++ = 0
                    }
                    else
                    {
                        add = 0;
                        De_fprintf(stderr,"new pthread_list_t fail\r\n");
                    }
                }
                else
                {
                    add = 0;
                    pool->threads->push_back(pid);
                }
            }

            pthread_mutex_unlock(&(pool->lock));

        }

        /* 销毁多余的空闲线程 算法：忙线程X2 小于 存活的线程数 且 存活的线程数 大于 最小线程数时*/
        if ((busy_thr_num * 2) < live_thr_num  &&  live_thr_num > pool->min_thr_num) {

            /* 一次销毁DEFAULT_THREAD个线程, 隨機10個即可 */
            pthread_mutex_lock(&(pool->lock));
            pool->wait_exit_thr_num = DEFAULT_THREAD_VARY;      /* 要销毁的线程数 设置为10 */
            pthread_mutex_unlock(&(pool->lock));

            for (i = 0; i < DEFAULT_THREAD_VARY; i++) {
                /* 通知处在空闲状态的线程, 他们会自行终止*/
                pthread_cond_signal(&(pool->queue_not_empty));

            }

        }

    }

    return NULL;

}

/*******************************************************************************
 * 名称: threadpool_free
 * 功能: 释放线程池结构内存
 * 形参: *pool:线程池结构体指针
 * 返回: 永远还回0
 * 说明: 无
 ******************************************************************************/
int threadpool_free(threadpool_t *pool)
{
    if (pool == NULL) {
        return -1;

    }

    if (pool->threadpool_task_queue) {
        for(thread_pool_task_queue_t::iterator i=pool->threadpool_task_queue->begin();
            i != pool->threadpool_task_queue->end();i++)
        {
            if((*i).arg != NULL)
            {
                free((*i).arg);
            }
        }
        delete pool->threadpool_task_queue;
    }
    if (pool->threads) {
        delete pool->threads;
        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_mutex_lock(&(pool->thread_counter));
        pthread_mutex_destroy(&(pool->thread_counter));
        pthread_cond_destroy(&(pool->queue_not_empty));
        pthread_cond_destroy(&(pool->queue_not_full));

    }
    free(pool);
    pool = NULL;

    return 0;

}
/*******************************************************************************
 * 名称: threadpool_destroy
 * 功能: 关闭线程池
 * 形参: *pool:线程池结构体指针
 * 返回: 成功还回0 pool==NULL还回-1
 * 说明: 无
 ******************************************************************************/
int threadpool_destroy(threadpool_t *pool)
{
    int i;
    if (pool == NULL) {
        return -1;

    }
    pool->shutdown = true;

    /*先销毁管理线程*/
    pthread_join(pool->adjust_tid, NULL);
    int pive_thr_num = pool->threads->size();
    for (i = 0; i < pive_thr_num; i++) {
        /*通知所有的空闲线程*/
        pthread_cond_broadcast(&(pool->queue_not_empty));

    }
    for(pthread_list_t::iterator i=pool->threads->begin(); i != pool->threads->end(); i++)
    {
        pthread_join(*(i), NULL);
    }
    threadpool_free(pool);


    return 0;

}
/*******************************************************************************
 * 名称: threadpool_all_threadnum
 * 功能: 获取当前线程池线程数目
 * 形参: *pool:线程结构指针
 * 返回: 当前线程数目
 * 说明: 无
 ******************************************************************************/
int threadpool_all_threadnum(threadpool_t *pool)
{
    int all_threadnum = -1;
    pthread_mutex_lock(&(pool->lock));
    all_threadnum = pool->threads->size();
    pthread_mutex_unlock(&(pool->lock));
    return all_threadnum;

}
/*******************************************************************************
 * 名称: threadpool_busy_threadnum
 * 功能: 还回忙碌线程数目
 * 形参: *pool:线程结构指针
 * 返回: 当前忙碌线程数
 * 说明: 无
 ******************************************************************************/
int threadpool_busy_threadnum(threadpool_t *pool)
{
    int busy_threadnum = -1;
    pthread_mutex_lock(&(pool->thread_counter));
    busy_threadnum = pool->busy_thr_num;
    pthread_mutex_unlock(&(pool->thread_counter));
    return busy_threadnum;

}




