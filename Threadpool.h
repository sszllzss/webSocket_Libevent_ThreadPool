/*************************************************************************
# > File Name: Threadpool.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-16 18:16:05
# > Revise Time: 2018-09-17 13:06:28
 ************************************************************************/

#ifndef _THREADPOOL_H
#define _THREADPOOL_H
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <deque>
#include "debug.h"
#include <list>
typedef struct threadpool_task_t threadpool_task_t;
typedef  std::deque<threadpool_task_t> thread_pool_task_queue_t;
typedef  std::list<pthread_t> pthread_list_t;
typedef struct threadpool_t threadpool_t;
threadpool_t *threadpool_create(int min_thr_num);//创建线程池
int threadpool_add(threadpool_t *pool, void*(*function)(void *arg), void *arg);//向线程池添加任务
int threadpool_destroy(threadpool_t *pool);//关闭并销毁线程池
int threadpool_all_threadnum(threadpool_t *pool);//获取线程池线程数
int threadpool_busy_threadnum(threadpool_t *pool);//获取线程池忙碌线程数
#endif
