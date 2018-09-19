/*************************************************************************
# > File Name: config.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-19 22:00:57
# > Revise Time: 2018-09-19 22:21:54
 ************************************************************************/

#ifndef _CONFIG_H
#define _CONFIG_H

#define DEBUG_PRINTF //打开调试信息

#define DEFAULT_TIME 500                 /*100ms检测一次*/
#define MIN_WAIT_TASK_NUM 10            /*如果queue_size > MIN_WAIT_TASK_NUM 添加新的线程到线程池*/ 
#define DEFAULT_THREAD_VARY 10          /*每次创建和销毁线程的个数*/
#define TH_CREATE_RETRY_NUM 2           /* 线程创建失败重试次数 */

#define BASE_MAX_MONITOR_NUM 1000//一个evbase监听最大事件数
#define BASE_MANAGER_TIMER 1000 //1s管理一次

#define REPORT_LOGIN_CONNECT_TIMEOUT      1000                                                                       // 登录连接超时设置 1000ms
#define REPORT_LOGIN_RESPOND_TIMEOUT      (1000 + REPORT_LOGIN_CONNECT_TIMEOUT)    // 登录等待回应超时设置 1000ms
#define REPORT_ANALYSIS_ERR_RESEND_DELAY    500     // 接收到回复内容但解析不通过, 延时 一段时间后重发指令      单位ms
#define WEBSOCKET_SHAKE_KEY_LEN     16

#endif