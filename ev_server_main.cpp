/*************************************************************************
# > File Name: ev_server_main.cpp
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-14 09:54:35
# > Revise Time: 2018-09-18 21:18:40
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
#include<event2/listener.h>
#include<event2/event-config.h>
#include"websocket_common.h"
#include <map>

#define SERVER_PORT 8088
#define BufferevMap  std::map<struct bufferevent *, sockaddr>
typedef int (*CallBackFun)(struct bufferevent * bufferev, char *buf, unsigned int bufLen, void * arg);
typedef struct{
    event_base * base;
    evconnlistener *listrner;
    BufferevMap *clientMap;
    int port;
    char buf[1024];
    CallBackFun action;
}WebSocket_server;
static int server_action(struct bufferevent * bufferev, char *buf, unsigned int bufLen, void * arg)
{
    int ret;
    WebSocket_server *ws=(WebSocket_server *)arg;
    ret = ev_webSocket_recv(bufferev ,(unsigned char *)buf , bufLen);    // 使用websocket recv
    if(ret > 0)
    {

        char ip[INET_ADDRSTRLEN];
        struct sockaddr_in *address = NULL; 
        BufferevMap::iterator i;
        i = ws->clientMap->find(bufferev);
        if(i!=ws->clientMap->end())
        {
            address = (struct sockaddr_in *)&(i->second);
        }
        if(address != NULL)
            printf("client %s:%d data len\\%d:%s\r\n", inet_ntop(AF_INET,&address->sin_addr, ip,(socklen_t) sizeof(ip)), ntohs(address->sin_port), ret , buf);  
        else
            printf("client Unknown data len\\%d:%s\r\n", ret, buf);

        if(strstr(buf, "connect") != NULL)     // 成功连上之后, 发个测试数据
            ret = ev_webSocket_send(bufferev,(unsigned char *) "Hello !", strlen("Hello !"), false, WCT_TXTDATA);
        else if(strstr(buf, "Hello") != NULL)
            ret = ev_webSocket_send(bufferev,(unsigned char *) "I am Server_Test", strlen("I am Server_Test"), false, WCT_TXTDATA);
        else
            ret = ev_webSocket_send(bufferev,(unsigned char *) "You are carefree ...", strlen("You are carefree ..."), false, WCT_TXTDATA);


    }
    return ret;
}
static void bufferev_read_cb(struct bufferevent *bev,void *ctx)
{
    WebSocket_server * ws = (WebSocket_server *)ctx;
    int ret;
    memset(ws->buf, 0, sizeof(ws->buf));
    ret = ws->action(bev, ws->buf, sizeof(ws->buf),ws);
    if(ret <0)
    {
        char ip[INET_ADDRSTRLEN];
        struct sockaddr_in *address = NULL; 
        BufferevMap::iterator i;
        i = ws->clientMap->find(bev);
        if(i!=ws->clientMap->end())
        {
            address = (struct sockaddr_in *)&(i->second);
        }
        if(address != NULL)
            printf("Connection close:%s:%d\r\n", inet_ntop(AF_INET,&address->sin_addr, ip,(socklen_t) sizeof(ip)), ntohs(address->sin_port));  
        else
            printf("Connection close: Unknown\r\n");
        bufferevent_disable(bev, EV_WRITE | EV_READ);
        ws->clientMap->erase(bev);
        bufferevent_free(bev);
    }
}
static void bufferev_event_cb(struct bufferevent *bev, short events,void *ctx)
{
    WebSocket_server *ws=(WebSocket_server *)ctx;
    struct sockaddr_in *address = NULL; 
    char ip[INET_ADDRSTRLEN];
    BufferevMap::iterator i;
    i = ws->clientMap->find(bev);
    if(i!=ws->clientMap->end())
    {
        address = (struct sockaddr_in *)&(i->second);
    }
    if(events & BEV_EVENT_ERROR)
    {
        if(address != NULL)
            printf("%s:%d[ Error from bufferevnt:%s\r\n", inet_ntop(AF_INET,&address->sin_addr, ip,(socklen_t) sizeof(ip)), ntohs(address->sin_port), strerror(errno));  
        else
            printf("Unknown[ Error from bufferevnt:%s\r\n",strerror(errno));
    }else if(events & (BEV_EVENT_EOF))
    {
        if(address != NULL)
            printf("Connection close:%s:%d\r\n", inet_ntop(AF_INET,&address->sin_addr, ip,(socklen_t) sizeof(ip)), ntohs(address->sin_port));  
        else
            printf("Connection close: Unknown\r\n");
    }

    ws->clientMap->erase(bev);
    bufferevent_disable(bev, EV_WRITE | EV_READ);
    bufferevent_free(bev);
}
static void  accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx)
{
    struct event_base *base = evconnlistener_get_base(listener);
    WebSocket_server * ws =(WebSocket_server  *)ctx;//(消除警告)
    char ip[INET_ADDRSTRLEN];
    struct sockaddr_in * addr = (struct sockaddr_in *)address;
    inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if(bev == NULL)
    {
        fprintf(stderr, "%s:%d[ bufferevent new error:%s\r\n",ip, ntohs(addr->sin_port), evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        return;
    }
    bufferevent_setcb(bev, bufferev_read_cb, NULL, bufferev_event_cb, ws);
    bufferevent_enable(bev, EV_READ| EV_WRITE);
    ws->clientMap->insert(std::pair<struct bufferevent *, sockaddr>(bev ,*address));
    printf("client connection:%s:%d\r\n",ip, ntohs(addr->sin_port));
    socklen = (int)socklen;
}
static void accpt_error_cb(struct evconnlistener *listener, void *ctx)
{
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    ctx =(void *)ctx;//(消除警告)
    fprintf(stderr, "Got an error %d (%s) the listener."
            "Shuttinf sown.\r\n",err, evutil_socket_error_to_string(err));
    event_base_loopexit(base, NULL);
}
static void signal_timer_out_cb(evutil_socket_t sig, short events, void *arg )
{
    if(sig == SIGALRM)
    {
        WebSocket_server *ws = (WebSocket_server *)arg;
        int ret;
        BufferevMap::iterator i;
        for(i = ws->clientMap->begin();i != ws->clientMap->end(); i++)
        {
            ret = ev_webSocket_send(i->first ,(unsigned char*) "\\O^O/  <-.<-  TAT  =.=#  -.-! ...", strlen("\\O^O/  <-.<-  TAT  =.=#  -.-! ..."), false, WCT_TXTDATA);

        }

    }
}
int main(int argc, char *argv[])
{
    struct event_base *base;
    struct event *signal_timer_out;
    WebSocket_server ws;
    struct evconnlistener *listener;
    struct sockaddr_in sin;
    int port = SERVER_PORT;
    struct itimerval timer_out;

    if(argc > 1)
    {
        port = atoi(argv[1]); 
    }
    if(port <=0 || port >65535)
    {
        printf("输入端口错误！\r\n");
        exit(1);
    }
    base = event_base_new();
    if(!base)
    {
        printf("base 创建失败！\r\n");
        exit(1);
    }

    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(0);
    sin.sin_port = htons(port);

    memset(&ws, 0,sizeof(ws));
    ws.clientMap = new BufferevMap();
    ws.port = port;
    ws.base = base;
    ws.action = server_action;
    listener = evconnlistener_new_bind(base, accept_conn_cb, &ws, LEV_OPT_CLOSE_ON_FREE| LEV_OPT_REUSEABLE| LEV_OPT_THREADSAFE, -1, (struct sockaddr *)&sin, sizeof(sin));
    if(!listener)
    {
        perror("监听失败：");
        delete  ws.clientMap;
        exit(1);
    }
    ws.listrner = listener;
    evconnlistener_set_error_cb(listener, accpt_error_cb);

    signal_timer_out = evsignal_new(base, SIGALRM, signal_timer_out_cb, &ws);
    if(!signal_timer_out || event_add(signal_timer_out, NULL)<0)
    {
        fprintf(stderr ,"Could not create/add a signal event!\r\n");
    }
    else
    {
        memset(&timer_out, 0, sizeof(timer_out));
        timer_out.it_value.tv_sec = 2;
        timer_out.it_value.tv_usec = 0;
        timer_out.it_interval.tv_sec = 2;
        timer_out.it_value.tv_usec = 0;
        setitimer(ITIMER_REAL , &timer_out, NULL);
    }

    event_base_dispatch(base);


    evconnlistener_free(listener);

    delete  ws.clientMap;
    event_base_free(base);
    return 0;
}
