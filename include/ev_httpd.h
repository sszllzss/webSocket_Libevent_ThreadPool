/*************************************************************************
# > File Name: Http_websocket/include/ev_httpd.h
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-24 13:53:55
# > Revise Time: 2018-10-15 15:47:33
 ************************************************************************/

#ifndef _EV_HTTPD_H

#define _EV_HTTPD_H
#include <stdio.h>
#include <iostream>
#include <map>
#include <arpa/inet.h>
#include "ev_websocket.h"
#define STRING std::string
#define HeadParameterMap  std::map<STRING, STRING>
#define HaedParameteMapPairi(key,value) std::pair<STRING, STRING>(key , value)
 /* 响应码  */
#define HTTP_SWITCHING_PROTOCOLS 101 /**< 协议升级成功  */
#define HTTP_SWITCHING_PROTOCOLS_VALUE "Switching Protocols"
#define HTTP_OK         200 /**< 请求完成  */
#define HTTP_OK_VALUE   "OK"
#define HTTP_NOCONTENT      204 /**< 请求没有内容 */
#define HTTP_NOCONTENT_VALUE   "No Content"
#define HTTP_MOVEPERM       301 /**< 资源（网页等）被永久转移到其它URL  */
#define HTTP_MOVEPERM_VALUE    "Moved Permanently"
#define HTTP_MOVETEMP       302 /**< 重定向和网址劫持 */
#define HTTP_MOVETEMP_VALUE    "Found"
#define HTTP_NOTMODIFIED    304 /**< 自从上次请求后，请求的网页未修改过*/
#define HTTP_NOTMODIFIED_VALUE "Not Modified"
#define HTTP_BADREQUEST     400 /**< 无效的http请求 */
#define HTTP_BADREQUEST_VALUE  "Bad Request"
#define HTTP_NOTFOUND       404 /**< 找不到uri的内容 */
#define HTTP_NOTFOUND_VALUE    "Not Found"
#define HTTP_BADMETHOD      405     /**< 方法禁用*/
#define HTTP_BADMETHOD_VALUE   "Method Not Allowed"
#define HTTP_ENTITYTOOLARGE 413 /**<  请求实体太大*/
#define HTTP_ENTITYTOOLARGE_VALUE "Request Entity Too Large"
#define HTTP_EXPECTATIONFAILED  417 /**< 预期结果失败 */
#define HTTP_EXPECTATIONFAILED_VALUE "Expectation Failed"
#define HTTP_INTERNAL           500     /**< 内部错误 */
#define HTTP_INTERNAL_VALUE "Internal Server Error"
#define HTTP_NOTIMPLEMENTED     501     /**< 服务器不具备完成请求的功能。例如,服务器无法识别请求方法时可能会返回此代码 */
#define HTTP_NOTIMPLEMENTED_VALUE "Not Implemented"
#define HTTP_SERVUNAVAIL    503 /**< 服务器不可用 */
#define HTTP_SERVUNAVAIL_VALUE "Service Unavailable"

typedef struct httpServer_t httpServer_t;
typedef struct httpChilent_t httpChilent_t;
typedef void (*httpd_handler_t)(struct httpChilent_t *chilent);
typedef void (*webSocket_read_cb_t)(struct httpChilent_t *chilent);
typedef struct http_request_t http_request_t;
typedef struct http_resqonse_t http_resqonse_t;
struct http_request_t
{
    char * method;
    char * query_string;
    char * url;
    char * http_version;
    HeadParameterMap *HeadParameter;
    char * request_data;
    unsigned request_data_len;
};
struct http_resqonse_t
{
    unsigned resqonseCode;
    HeadParameterMap *HeadParameter;
    char * resqonse_data;
    unsigned Resqonse_data_len;
};
enum httpChilentStat_t
{
    HTTP_DISCONNECTED = 0,/*未连接*/ 
    HTTP_CONNECTED,/*客户端连接*/
    HTTP_IDLE,/* 空闲状态 */
    HTTP_REQUEST,/* 读取请求  */
    HTTP_RESQONSE,/* 响应请求 */
    HTTP_WS_DATA,/* WebSocket  */
};
struct httpChilent_t
{
    pthread_mutex_t lock;
    httpServer_t *httpServer;
    struct bufferevent *bev; 
    httpChilentStat_t stat;  
    http_request_t request;
    http_resqonse_t resqonse;
    struct evbuffer *receive_evbuff;//读取缓存
    Websocket_CommunicationType ws_data_type;
    struct sockaddr addr;
    struct timeval final_optime;
};
/* resqonse这个函数 内部 对 client->lock进行了加锁 请在调用时 解锁 client->lock */
int resqonse(struct httpChilent_t *client);
httpServer_t * startHttpServe(struct event_base* base, u_short *port);
int httpServer_free(httpServer_t * httpServer);
int http_resqonse_free(struct http_resqonse_t *resqonse);
int http_request_free(struct http_request_t *request);
void httpChilent_Close(struct httpChilent_t * client);
void httpChilent_Close_nolock(struct httpChilent_t * client);
int httpServer_setWebSocket_read_cb(httpServer_t * httpServer, webSocket_read_cb_t ws_read_cb,const char * url);
int httpServer_setHttpHandler(httpServer_t *httpServer, httpd_handler_t handler);
void httpServer_setArg(httpServer_t *httpServer, void * arg);
void* httpServer_getArg(httpServer_t *httpServer);
#endif

