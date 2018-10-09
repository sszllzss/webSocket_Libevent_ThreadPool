/*************************************************************************
# > File Name: include/config.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-19 22:00:57
# > Revise Time: 2018-10-09 15:18:39
 ************************************************************************/

#ifndef _CONFIG_H
#define _CONFIG_H
#define DYNAMIC_CONFING
#define DEBUG_PRINTF //打开调试信息
#ifndef DYNAMIC_CONFING
//Threaadpool.cpp
#define DEFAULT_TIME 100                 /*100ms检测一次*/
#define MIN_WAIT_TASK_NUM 20            /*如果queue_size > MIN_WAIT_TASK_NUM 添加新的线程到线程池*/ 
#define DEFAULT_THREAD_VARY 10          /*每次创建和销毁线程的个数*/
#define TH_CREATE_RETRY_NUM 2           /* 线程创建失败重试次数 */
//evbade_threaadpool.cpp
#define BASE_MAX_MONITOR_NUM 1000//一个evbase监听最大事件数 默认次数
#define BASE_MANAGER_TIMER 100 //100ms管理一次
//
#define REPORT_LOGIN_CONNECT_TIMEOUT      1000                                                                       // 登录连接超时设置 1000ms
#define REPORT_LOGIN_RESPOND_TIMEOUT      (1000 + REPORT_LOGIN_CONNECT_TIMEOUT)    // 登录等待回应超时设置 1000ms
#define REPORT_ANALYSIS_ERR_RESEND_DELAY    500     // 接收到回复内容但解析不通过, 延时 一段时间后重发指令      单位ms
#define WEBSOCKET_SHAKE_KEY_LEN     16
//ev_httpd.cpp
#define RECV_DATA_TIME_OUT 20 //接收数据超时 20次接收为0

#define HTTPSERVER_MIN_THREAD_NUM 30 
#define HTTPSERVER_BASE_MAX_MONITOR_NUM 2000
#define HTTPSERVER_SUPERVISE_TIME 100 //100ms 
#define HTTPSERVER_IDLE_TIMEROUT 5 //5s
#define HTTPSERVER_RECEIVE_BUFF_CHUNK_SIZE 256 //256kb
#define SERVER_STRING "WebSocket-HttpServer/1.0.0"

#else
//Threaadpool.cpp
extern int DEFAULT_TIME_VARIABLE; 
#define DEFAULT_TIME DEFAULT_THREAD_VARY                 /*线程池管理者多久检测一次 单位：ms*/
extern unsigned MIN_WAIT_TASK_NUM_VARIABLE;            
#define MIN_WAIT_TASK_NUM MIN_WAIT_TASK_NUM_VARIABLE /*如果queue_size > MIN_WAIT_TASK_NUM 添加新的线程到线程池*/ 
extern int DEFAULT_THREAD_VARY_VARIABLE;          
#define DEFAULT_THREAD_VARY DEFAULT_THREAD_VARY_VARIABLE/*每次创建和销毁线程的个数*/
extern int TH_CREATE_RETRY_NUM_VARIABLE;
#define TH_CREATE_RETRY_NUM TH_CREATE_RETRY_NUM_VARIABLE/* 线程创建失败重试次数 */
//evbade_threaadpool.cpp
extern unsigned BASE_MAX_MONITOR_NUM_VARIABLE;
#define BASE_MAX_MONITOR_NUM BASE_MAX_MONITOR_NUM_VARIABLE//一个evbase监听最大事件数 默认次数
extern int BASE_MANAGER_TIMER_VARIABLE;
#define BASE_MANAGER_TIMER BASE_MANAGER_TIMER_VARIABLE//base监听池管理线程多久检测一次 单位：ms

extern int REPORT_LOGIN_CONNECT_TIMEOUT_VARIABLE;                                                                      // 登录连接超时设置 1000ms
#define REPORT_LOGIN_CONNECT_TIMEOUT REPORT_LOGIN_CONNECT_TIMEOUT_VARIABLE //websocket连接超时 // 登录连接超时设置 单位:ms
extern int REPORT_LOGIN_RESPOND_TIMEOUT_VARIABLE;
#define REPORT_LOGIN_RESPOND_TIMEOUT REPORT_LOGIN_RESPOND_TIMEOUT_VARIABLE// 登录等待回应超时设置 单位：ms
extern int REPORT_ANALYSIS_ERR_RESEND_DELAY_VARIALE;    
#define REPORT_ANALYSIS_ERR_RESEND_DELAY REPORT_ANALYSIS_ERR_RESEND_DELAY_VARIALE // 接收到回复内容但解析不通过, 延时 一段时间后重发指令      单位ms
#define WEBSOCKET_SHAKE_KEY_LEN 16//连接密匙长度
//ev_httpd.cpp
extern unsigned  RECV_DATA_TIME_OUT_VARIALE;
#define RECV_DATA_TIME_OUT RECV_DATA_TIME_OUT_VARIALE//接收数据超时 
extern int HTTPSERVER_MIN_THREAD_NUM_VARIALE;//Http服务器线程池最小进程数 
#define HTTPSERVER_MIN_THREAD_NUM HTTPSERVER_MIN_THREAD_NUM_VARIALE 
extern int HTTPSERVER_BASE_MAX_MONITOR_NUM_VARIALE;//一个base最大监听数
#define HTTPSERVER_BASE_MAX_MONITOR_NUM HTTPSERVER_BASE_MAX_MONITOR_NUM_VARIALE
extern int HTTPSERVER_SUPERVISE_TIME_VARIALE;//http服务器连接监管时间 
#define HTTPSERVER_SUPERVISE_TIME HTTPSERVER_SUPERVISE_TIME_VARIALE
extern int HTTPSERVER_IDLE_TIMEROUT_VARIALE;//连接空闲时间 当一个连接一直处于空闲状态数据超过这个值断开连接 单位：s
#define HTTPSERVER_IDLE_TIMEROUT HTTPSERVER_IDLE_TIMEROUT_VARIALE
extern int HTTPSERVER_RECEIVE_BUFF_CHUNK_SIZE_VARIALE;//一次分配内存块大小 单位：kb
#define HTTPSERVER_RECEIVE_BUFF_CHUNK_SIZE HTTPSERVER_RECEIVE_BUFF_CHUNK_SIZE_VARIALE
extern char * SERVER_STRING_VARIALE;//http服务器名字
#define SERVER_STRING  SERVER_STRING_VARIALE
void set_SERVER_STRING(char * name);

#endif

#endif
