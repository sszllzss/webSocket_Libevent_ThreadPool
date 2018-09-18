/*************************************************************************
# > File Name: ev_thread_server_main.cpp
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-14 09:54:35
# > Revise Time: 2018-09-17 22:09:33
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
#include <map>
#include <vector>
#define BASE_MAX_MONITOR_NUM 1000//一个evbase监听最大事件数
typedef struct
{
    pthread_mutex_t monitor_num_locl;//监听数锁
    event_base* base;
    pthread_t pid;
    unsigned monitor_num;//事件监听数
}evbase_thread_t;
typedef struct{
    pthread_mutex_t lock;//结构体锁
    std::vector<evbase_thread_t*> *list;
}evbase_thread_list;



