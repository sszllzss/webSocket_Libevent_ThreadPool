/*************************************************************************
# > File Name: websocket_common.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-14 09:54:35
# > Revise Time: 2018-10-01 15:20:16
 ************************************************************************/


#ifndef _WEBSOCKET_COMMON_H_
#define _WEBSOCKET_COMMON_H_

#include <stdio.h>   
#include <stdlib.h>  
#include <string.h>     // 使用 malloc, calloc等动态分配内存方法
#include <stdbool.h>
#include <time.h>       // 获取系统时间
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>                      // socket设置非阻塞模式
#include <arpa/inet.h>
#include <assert.h>
#define OPENSSL
#define LIBEVENT
#ifdef OPENSSL 
#include <openssl/sha.h>
#endif
#ifdef LIBEVENT 
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include<event2/util.h>
#include<event2/dns.h>
#endif
// websocket根据data[0]判别数据包类型    比如0x81 = 0x80 | 0x1 为一个txt类型数据包
typedef enum{
    WCT_MINDATA = -20,      // 0x0：标识一个中间数据包
    WCT_TXTDATA = -19,      // 0x1：标识一个txt类型数据包
    WCT_BINDATA = -18,      // 0x2：标识一个bin类型数据包
    WCT_DISCONN = -17,      // 0x8：标识一个断开连接类型数据包
    WCT_PING = -16,     // 0x8：标识一个断开连接类型数据包
    WCT_PONG = -15,     // 0xA：表示一个pong类型数据包
    WCT_ERR = -1,
    WCT_NULL = 0
}Websocket_CommunicationType;

int webSocket_clientLinkToServer(char *ip, int port, char *interface_path);
int webSocket_serverLinkToClient(int fd, char *recvBuf, unsigned int bufLen);

int webSocket_send(int fd, unsigned char *data, unsigned int dataLen, bool mod, Websocket_CommunicationType type);
int webSocket_recv(int fd, unsigned char *data, unsigned int dataMaxLen);

void delayms(unsigned int ms);
#ifdef LIBEVENT 
struct bufferevent *  ev_webSocket_clientLinkToServer(struct event_base * base,  char *ip, int port, char *interface_path, bufferevent_data_cb read_cb, bufferevent_data_cb write_cb, bufferevent_event_cb event_cb, void * arg);
int ev_webSocket_serverLinkToClient(struct bufferevent * client, char *recvBuf, unsigned int bufLen);

int ev_webSocket_send(struct bufferevent * bufferev, unsigned char *data, unsigned int dataLen, bool mod, Websocket_CommunicationType type);
int ev_webSocket_recv(struct bufferevent * bufferev, unsigned char *data, unsigned int dataMaxLen);
#endif
#endif

