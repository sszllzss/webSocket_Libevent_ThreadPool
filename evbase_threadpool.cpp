/*************************************************************************
# > File Name: evbase_threadpool.cpp
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-14 09:54:35
# > Revise Time: 2018-09-19 00:13:29
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<signal.h>
#include<sys/time.h>
#include <arpa/inet.h>
#include<event.h>
#include<event2/buffer.h>
#include<event2/bufferevent.h>
#include<event2/event.h>
#include<event2/event-config.h>
#include"websocket_common.h"
#include <vector>
#include "Threadpool.h"
#define BASE_MAX_MONITOR_NUM 1000//一个evbase监听最大事件数
#define BASE_MANAGER_TIMER 1000 //1s管理一次
typedef struct
{
    pthread_mutex_t monitor_num_locl;//监听数锁
    event_base* base;
    unsigned monitor_num;//事件监听数
}evbase_thread_t;
typedef  std::vector<evbase_thread_t*> evbase_thread_vector;
typedef struct{
    pthread_mutex_t lock;//结构体锁
    evbase_thread_vector *list;
}evbase_thread_list_t;
struct evbase_threadpool_t{
    threadpool_t *pool;
    evbase_thread_list_t evbase_thread_list;
    pthread_t manager;//监听线程管理者线程

};
struct evbase_thread_arg
{
    event_base * base;
    evbase_threadpool_t *evb_thpool;
};
void evbase_threadpool_destroy(evbase_threadpool_t * evb_thpool);
void *manager_thread(void *evbase_threadpool);
void *evbase_thread(void *arg);
/*******************************************************************************
 * 名称: evbase_threadpool_new
 * 功能: 创建evbase监听线程池
 * 形参: *base_max_monitor_num:一个base最大监听数目 传0 为默认值 BASE_MAX_MONITOR_NUM
 *       *min_thread_num:线程池最小线程数 必须大于0否则创建失败
 * 返回: 成功还回evbase_thread_t指针失败还回NULL
 * 说明: 无
 ******************************************************************************/
evbase_threadpool_t *evbase_threadpool_new(unsigned base_max_monitor_num,unsigned min_thread_num)
{
    if(base_max_monitor_num <= 0)
        base_max_monitor_num = BASE_MAX_MONITOR_NUM;
    if(min_thread_num<1)
        return NULL;
    evbase_threadpool_t *evbase_threadpool=NULL;
    do{
        evbase_threadpool = (evbase_threadpool_t *)malloc(sizeof(evbase_threadpool_t));
        if(evbase_threadpool == NULL)
            break;
        memset(evbase_threadpool,0, sizeof(evbase_threadpool_t));
        evbase_threadpool->pool = threadpool_create(min_thread_num);
        if(evbase_threadpool->pool == NULL)
            break;
        evbase_threadpool->evbase_thread_list.list = new evbase_thread_vector();
        if(evbase_threadpool->evbase_thread_list.list == NULL)
            break;
        if(pthread_mutex_init(&evbase_threadpool->evbase_thread_list.lock, NULL) != 0)
        {
            break;
        }

        pthread_t pid=0;
        int ret = pthread_create(&pid, NULL, manager_thread, evbase_threadpool);
        if(ret!=0)
            break;
        evbase_threadpool->manager = pid;
        struct evbase_thread_arg *evb_th_arg = (struct evbase_thread_arg *)malloc(sizeof(struct evbase_thread_arg));
        if(evb_th_arg == NULL)
            break;
        memset(evb_th_arg, 0, sizeof(struct evbase_thread_arg));
        evb_th_arg->base = event_base_new();
        if(evb_th_arg == NULL)
            break;
        evb_th_arg->evb_thpool = evbase_threadpool;
        evbase_thread_t *evb_th = (evbase_thread_t *)malloc(sizeof(evbase_thread_t));
        if(evb_th == NULL)
            break;
        memset(evb_th, 0, sizeof(evbase_thread_t));
        evb_th->base = evb_th_arg->base;
        if(pthread_mutex_init(&evb_th->monitor_num_locl, NULL) != 0)
            break;

        evbase_threadpool->evbase_thread_list.list->push_back(evb_th);
        //创建一个base监听
        threadpool_add(evbase_threadpool->pool, evbase_thread, evb_th_arg);
    }while(0);
    evbase_threadpool_destroy(evbase_threadpool);
    return NULL;
}

/*******************************************************************************
 * 名称: evbase_threadpool_free
 * 功能: 回收资源
 * 形参: 
 * 返回: 
 * 说明: 无
 ******************************************************************************/
void evbase_threadpool_free(evbase_threadpool_t * evb_thpool)
{
    if(evb_thpool == NULL)
        return;
    if(evb_thpool->evbase_thread_list.list != NULL)
    {
        for(evbase_thread_vector::iterator i = evb_thpool->evbase_thread_list.list->begin();
            i != evb_thpool->evbase_thread_list.list->end();i++ )
        {
            if((*i)->base != NULL)
                event_base_free((*i)->base);
            pthread_mutex_destroy(&(*i)->monitor_num_locl);    
            free(*i);
        }
        delete evb_thpool->evbase_thread_list.list;
    }
    pthread_mutex_destroy(&evb_thpool->evbase_thread_list.lock);
    free(evb_thpool);
}
/*******************************************************************************
 * 名称: evbase_threadpool_free
 * 功能: 关闭evbase监听线程池
 * 形参: *evb_thpool:evbase_threadpool_t指针
 * 返回: 
 * 说明: 无
 ******************************************************************************/
void evbase_threadpool_destroy(evbase_threadpool_t * evb_thpool)
{
    if(evb_thpool == NULL)
        return;
    if(evb_thpool->manager != 0)
    {
        pthread_join(evb_thpool->manager, NULL);
    }
    if(evb_thpool->pool != NULL)
    {
        threadpool_destroy(evb_thpool->pool);
    }
    evbase_threadpool_free(evb_thpool);
}
/*******************************************************************************
 * 名称: evbase_threadpool_add_event
 * 功能: 向evbase池申请base 对应 base监听数加1
 * 形参: 
 * 返回: event_base指针 NULL 申请失败
 * 说明: 无
 ******************************************************************************/
event_base * evbase_threadpool_add_event(evbase_threadpool_t * evb_thpool)
{
    event_base *base = NULL;
    evbase_thread_vector::iterator i;
    for(i = evb_thpool->evbase_thread_list.list->begin();
        i != evb_thpool->evbase_thread_list.list->end(); i++)
    {
        pthread_mutex_lock(&(*i)->monitor_num_locl);
        if((*i)->monitor_num < BASE_MAX_MONITOR_NUM)
        {
            base = (*i)->base;
            (*i)->monitor_num++;
            break;
        }
        pthread_mutex_unlock(&(*i)->monitor_num_locl);
    }
    if(i == evb_thpool->evbase_thread_list.list->end())
    {
        do{
            struct evbase_thread_arg *evb_th_arg = (struct evbase_thread_arg *)malloc(sizeof(struct evbase_thread_arg));
            if(evb_th_arg == NULL)
                break;
            memset(evb_th_arg, 0, sizeof(struct evbase_thread_arg));
            evb_th_arg->base = event_base_new();
            if(evb_th_arg->base == NULL)
            {
                free(evb_th_arg);
                break;
            }
            evb_th_arg->evb_thpool = evb_thpool;

            evbase_thread_t *evb_th = (evbase_thread_t *)malloc(sizeof(evbase_thread_t));
            if(evb_th == NULL)
            {
                event_base_free(evb_th_arg->base);
                free(evb_th_arg);
                break;
            }
            memset(evb_th, 0, sizeof(evbase_thread_t));
            evb_th->base = base;
            evb_th->monitor_num++;
            if(pthread_mutex_init(&evb_th->monitor_num_locl, NULL) != 0)
            {
                free(evb_th);
                event_base_free(evb_th_arg->base);
                free(evb_th_arg);
                break;
            }
            pthread_mutex_lock(&evb_thpool->evbase_thread_list.lock);
            evb_thpool->evbase_thread_list.list->push_back(evb_th);
            pthread_mutex_unlock(&evb_thpool->evbase_thread_list.lock);

            //创建一个base监听
            threadpool_add(evb_thpool->pool, evbase_thread, evb_th_arg);
            base = evb_th_arg->base;
        }while(0);   
    }

    return base;
}
/*******************************************************************************
 * 名称: evbase_threadpool_close_event
 * 功能: 已经关闭了 base 上的一个事件 base监听数减1
 * 形参: *base:event_base指针
 * 返回: 
 * 说明: 无
 ******************************************************************************/
void evbase_threadpool_close_event(evbase_threadpool_t *evb_thpool ,event_base * base)
{

    for(evbase_thread_vector::iterator i = evb_thpool->evbase_thread_list.list->begin();
        i != evb_thpool->evbase_thread_list.list->end(); i++)
    {
        if((*i)->base == base)
        {
            if((*i)->monitor_num >0)
                (*i)->monitor_num--;
            else
                De_fprintf(stderr, "base监听数为0时发生了 coles_event\r\n");
        }
    }           

}
/*******************************************************************************
 * 名称: manager_thread 
 * 功能: evb_thpool管理者线程 结束base监听线程
 * 形参: evbase_threadpool:evbase池结构指针
 * 返回: 
 * 说明: 无
 ******************************************************************************/
void *manager_thread(void *evbase_threadpool)
{
    evbase_threadpool_t *evb_thpool = (evbase_threadpool_t*)evbase_threadpool; 
    while(true)
    {
        usleep(BASE_MANAGER_TIMER*1000);        
        pthread_mutex_lock(&evb_thpool->evbase_thread_list.lock);
        for(evbase_thread_vector::iterator i = evb_thpool->evbase_thread_list.list->begin();
            i != evb_thpool->evbase_thread_list.list->end(); )
        {
            pthread_mutex_lock(&(*i)->monitor_num_locl);    
            if((*i)->monitor_num == 0 && evb_thpool->evbase_thread_list.list->size() > 1){
                if(!event_base_loopbreak((*i)->base))
                {
                    event_base_dispatch((*i)->base);
                    pthread_mutex_destroy(&(*i)->monitor_num_locl);
                    evb_thpool->evbase_thread_list.list->erase(i);
                    free(*i);
                }
                else
                {
                    i++;
                    pthread_mutex_unlock(&(*i)->monitor_num_locl);    
                }
            }
            else
            {
                i++;
                pthread_mutex_unlock(&(*i)->monitor_num_locl);    
            }
        }
        pthread_mutex_unlock(&evb_thpool->evbase_thread_list.lock);
    }
}
/*******************************************************************************
 * 名称: evbase_threadpool
 * 功能: evbase监听进程
 * 形参: evbase_threadpool:evbase池结构指针
 * 返回: 
 * 说明: 无
 ******************************************************************************/
void *evbase_thread(void *arg)
{
    struct evbase_thread_arg * evb_th_arg = (struct evbase_thread_arg *)arg;    
    struct event_base *base = evb_th_arg->base;
    //回收内存
    free(arg);

    event_base_dispatch(base);

    event_base_free(base);

    return NULL;
}
