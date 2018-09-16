/*************************************************************************
# > File Name: ev_client_main.c
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-16 12:08:15
# > Revise Time: 2018-09-16 16:57:53
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<signal.h>
#include<event2/util.h>
#include<event2/dns.h>
#include<assert.h>
#include<sys/time.h>
#include"websocket_common.h"
#define SERVER_PORT 8088
#define SERVERR_IP "lot.sszlsszl.com"
#define INADDR_LEN 50
static void bev_read_cd(struct bufferevent *bev, void *ctx)
{
    ctx = (void*)ctx;
    char buf[1024];
    int ret = ev_webSocket_recv(bev, (unsigned char*)buf, sizeof(buf));
    if(ret>0)
    {
        printf("client recv : len/%d %s\r\n", ret, buf);
        if(strstr(buf, "Hello") != NULL)
            ret = ev_webSocket_send(bev, (unsigned char *)"I am Client_Test", strlen("I am Client_Test"), true, WCT_TXTDATA);
        else if(strstr(buf, "Server_Test") != NULL)
            ret = ev_webSocket_send(bev, (unsigned char *)"I am carefree !", strlen("I am carefree !"), true, WCT_TXTDATA);
    }
    else
    {
        De_printf("断开连接!\r\n");
        event_base_loopbreak(bufferevent_get_base(bev));
        bufferevent_free(bev);
    }
}
static void bev_event_cd(struct bufferevent *bev,short events, void *ctx)
{
    ctx = (void*)ctx;
    if(events & BEV_EVENT_EOF)
        De_printf("断开连接!\r\n");
    else if(events & BEV_EVENT_ERROR)
    {
        De_printf("Got an error on the connection:%s\r\n", strerror(errno));
    }
    event_base_loopbreak(bufferevent_get_base(bev));
    bufferevent_free(bev);

}
static void sig_cb(evutil_socket_t sig, short events,void *ctx)
{
    int ret;
    if(sig==SIGALRM)
    {
        struct bufferevent *bev=(struct bufferevent *)ctx;
        ret = ev_webSocket_send(bev,(unsigned char *) "#%^#@@@DTG%^&&+_)+(*^%!HHI", strlen("#%^#@@@DTG%^&&+_)+(*^%!HHI"), true, WCT_TXTDATA);
        if(ret < 0)
        {
            
            De_printf("断开连接!\r\n");
            event_base_loopbreak(bufferevent_get_base(bev));
            bufferevent_free(bev);
        }
    }
}
int main(int argc, char *argv[])
{
    char inAddr[INADDR_LEN];
    int port = 0;
    struct event_base *base=NULL;
    struct bufferevent *bev=NULL;
    struct event * sig=NULL;
    struct itimerval timer_out;
    if(argc==3)
    {
        if(strlen(argv[1])>INADDR_LEN)
        {
            printf("输入长度要少于%d\r\n", INADDR_LEN);
            exit(1);
        }
        strcpy(inAddr, argv[1]);
        port =  atoi(argv[2]);
        if(port==0)
        {
            printf("端口输入错误！！\r\n");
            exit(1);
        }
    }
    else
    {
        port = SERVER_PORT;
        strcpy(inAddr, SERVERR_IP);
    }
    base = event_base_new();
    assert(base);

    bev = ev_webSocket_clientLinkToServer(base, inAddr, port,(char *)"/null",bev_read_cd,NULL,bev_event_cd,NULL);
    if(!bev)
    {
        printf("连接服务器错误!\r\n");
        exit(1);
    }
    sig =  evsignal_new(base, SIGALRM, sig_cb, bev);
    if(!sig || event_add(sig, NULL))
    {
        De_fprintf(stderr, "cold not create/add a signal event!");
    }
    else
    {
        memset(&timer_out, 0, sizeof(timer_out));
        timer_out.it_interval.tv_sec = 2;
        timer_out.it_value.tv_sec = 2;
        setitimer(ITIMER_REAL, &timer_out, NULL);
    }
    event_base_dispatch(base);
    return 0;
}
