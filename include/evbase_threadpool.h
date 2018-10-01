/*************************************************************************
# > File Name: evbase_threadpool.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-17 22:10:26
# > Revise Time: 2018-10-01 15:18:08
 ************************************************************************/

#ifndef _EVBASE_THREADPOOL_H
#define _EVBASE_THREADPOOL_H
#include<event.h>
#include<event2/buffer.h>
#include<event2/bufferevent.h>
#include<event2/event.h>
#include<event2/event-config.h>
#include<arpa/inet.h>
#include<signal.h>
#include"Threadpool.h"
typedef struct evbase_threadpool_t  evbase_threadpool_t;
evbase_threadpool_t *evbase_threadpool_new(unsigned base_max_monitor_num,unsigned min_thread_num);
void evbase_threadpool_destroy(evbase_threadpool_t * evb_thpool);
event_base * evbase_threadpool_add_event(evbase_threadpool_t * evb_thpool);
void evbase_threadpool_close_event(evbase_threadpool_t *evb_thpool ,event_base * base);
threadpool_t *evbase_threadpool_get_threadpool(evbase_threadpool_t *ev_thpool);
#endif
