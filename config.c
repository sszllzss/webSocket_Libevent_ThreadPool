/*************************************************************************
# > File Name: config.c
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-09 13:33:44
# > Revise Time: 2018-10-09 15:19:00
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include"include/config.h"
//Threaadpool.cpp
int DEFAULT_TIME_VARIABLE = 100; /*100ms检测一次*/ /*线程池管理者多久检测一次 单位：ms*/
unsigned MIN_WAIT_TASK_NUM_VARIABLE = 20;/* 任务队列中任务数大于改值添加新的线程 */            
int DEFAULT_THREAD_VARY_VARIABLE = 10; /*每次创建和销毁线程的个数*/         
int TH_CREATE_RETRY_NUM_VARIABLE = 2;/* 线程创建失败重试次数 */
//evbade_threaadpool.cpp
unsigned BASE_MAX_MONITOR_NUM_VARIABLE = 1000;//一个evbase监听最大事件数 默认次数
int BASE_MANAGER_TIMER_VARIABLE = 100;//100ms管理一次 base监听池管理线程多久检测一次 单位：ms
int REPORT_LOGIN_CONNECT_TIMEOUT_VARIABLE = 1000; //websocket连接超时 登录连接超时设置 1000ms
int REPORT_LOGIN_RESPOND_TIMEOUT_VARIABLE = REPORT_LOGIN_CONNECT_TIMEOUT_VARIABLE + 1000;// 登录等待回应超时设置 1000m
int REPORT_ANALYSIS_ERR_RESEND_DELAY_VARIALE = 500;// 接收到回复内容但解析不通过, 延时 一段时间后重发指令      单位ms

//ev_httpd.cpp
unsigned  RECV_DATA_TIME_OUT_VARIALE = 20;//接收数据超时 20次接收为0
int HTTPSERVER_MIN_THREAD_NUM_VARIALE = 10;//Http服务器线程池最小进程数 
int HTTPSERVER_BASE_MAX_MONITOR_NUM_VARIALE = 1000;//一个base最大监听数
int HTTPSERVER_SUPERVISE_TIME_VARIALE = 100;//http服务器连接监管时间 单位：ms
int HTTPSERVER_IDLE_TIMEROUT_VARIALE = 5;//连接空闲时间 当一个连接一直处于空闲状态数据超过这个值断开连接 单位：s
int HTTPSERVER_RECEIVE_BUFF_CHUNK_SIZE_VARIALE = 256;//一次分配内存块大小 单位：kb
char * SERVER_STRING_VARIALE = NULL;//http服务器名字
void set_SERVER_STRING(char * name)
{
    if(SERVER_STRING_VARIALE == NULL)
    {
        SERVER_STRING_VARIALE = (char *)malloc(strlen(name));
    }
    else
    {
        SERVER_STRING_VARIALE = (char *) realloc(SERVER_STRING_VARIALE, strlen(name));
    }
    if(name == NULL)
        strcpy(SERVER_STRING_VARIALE,"WebSocket-HttpServer/1.0.0");
    else
        strcpy(SERVER_STRING_VARIALE, name);
}
