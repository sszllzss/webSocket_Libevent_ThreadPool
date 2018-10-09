/*************************************************************************
# > File Name: Http_websocket/ev_httpd.cpp
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-09-20 18:16:10
# > Revise Time: 2018-10-09 14:52:01
 ************************************************************************/
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/time.h>
#include <event2/listener.h>
#include <map>
#include <string>
#include <iostream>
#include "include/evbase_threadpool.h"
#include "include/debug.h"
#include "include/ev_websocket.h"
#include "include/ev_httpd.h"
#include "include/ev_httpd_content.h"
#include "include/config.h"
#ifndef HTTPSERVER_MIN_THREAD_NUM 
#define HTTPSERVER_MIN_THREAD_NUM 10 
#endif
#ifndef HTTPSERVER_BASE_MAX_MONITOR_NUM 
#define HTTPSERVER_BASE_MAX_MONITOR_NUM 3000
#endif
#ifndef HTTPSERVER_SUPERVISE_TIME  
#define HTTPSERVER_SUPERVISE_TIME 1000 //1s 
#endif
#ifndef HTTPSERVER_IDLE_TIMEROUT 
#define HTTPSERVER_IDLE_TIMEROUT 15 //10s
#endif
#ifndef HTTPSERVER_RECEIVE_BUFF_CHUNK_SIZE 
#define HTTPSERVER_RECEIVE_BUFF_CHUNK_SIZE 256 //256kb
#endif
#ifndef SERVER_STRING 
#define SERVER_STRING "WebSocket-HttpServer/1.0.0"
#endif
#ifndef METHOD_MAX_LEN
#define METHOD_MAX_LEN 125
#endif
#ifndef URL_MAX_LEN
#define URL_MAX_LEN 125
#endif
#ifndef HTTPVERSION_MAX_LEN
#define HTTPVERSION_MAX_LEN 10
#endif

#define BufferevMap  std::map<struct bufferevent *, struct httpChilent_t *>
#define ISspace(x) isspace((int)(x))

typedef enum
{
    REQUEST_ERROR = -1,
    REQUEST_NOCONTENT = -2,
    REQUEST_BADREQUEST = -3,
    REQUEST_ENTITYTOOLARGE = -4,
    REQUEST_INTERNAL = -5,
    REQUEST_NOTIMPLEMENTED = -6

}Accept_Request_Error;


struct httpServer_t
{
    pthread_mutex_t lock;
    pthread_mutex_t clientMap_lock;
    BufferevMap *clientMap;
    event_base * http_listener_base;
    evbase_threadpool_t * evb_thpool;
    evconnlistener * http_listeber;
    threadpool_t * pool;
    pthread_t http_supervise_thread;
    int port;
    int shutdown;
    httpd_handler_t handler;
    webSocket_read_cb_t ws_read_cb;
    char *webSocket_url;
};

int httpChilent_free(struct httpChilent_t* client);
int accept_request(struct httpChilent_t * client);


int resqonse(struct httpChilent_t *client);
httpServer_t * startHttpServe(struct event_base* base, u_short *port);
int httpServer_free(httpServer_t * httpServer);
int http_resqonse_free(struct http_resqonse_t *resqonse);
int http_request_free(struct http_request_t *request);
void httpChilent_Close(struct httpChilent_t * client);
void httpChilent_Close_nolock(struct httpChilent_t * client);
int httpServer_setWebSocket_read_cb(httpServer_t * httpServer, webSocket_read_cb_t ws_read_cb,const char * url);
int httpServer_setHttpHandler(httpServer_t *httpServer, httpd_handler_t handler);



int resqonse(struct httpChilent_t *client)
{
    struct evbuffer * evbuff = evbuffer_new();
    HeadParameterMap::iterator v;
    if(evbuff == NULL)
    {
        return -1;
    }
    pthread_mutex_lock(&client->lock);
    gettimeofday(&client->final_optime, NULL);
    do{
        if(client->resqonse.resqonseCode == 0)
        {
            pthread_mutex_unlock(&client->lock);
            return -1;
        }
        if(client->request.http_version == NULL)
        {
            client->request.http_version = (char *)malloc(strlen("HTTP/1.1"));
            if(client->request.http_version == NULL)
            {
                pthread_mutex_unlock(&client->lock);
                return -1;
            }
            strcpy(client->request.http_version,"HTTP/1.1");
        }
        if(client->resqonse.resqonseCode ==  HTTP_OK )
        {
            evbuffer_add_printf(evbuff, "%s %d %s\r\n", client->request.http_version, HTTP_OK, HTTP_OK_VALUE);
        }
        else if(client->resqonse.resqonseCode == HTTP_SWITCHING_PROTOCOLS)
        {
            evbuffer_add_printf(evbuff, "%s %d %s\r\n", client->request.http_version, HTTP_SWITCHING_PROTOCOLS, HTTP_SWITCHING_PROTOCOLS_VALUE);
            evbuffer_add_printf(evbuff, "Upgrade: websocket\r\n");
            evbuffer_add_printf(evbuff, "Connection: Upgrade\r\n");
            v = client->resqonse.HeadParameter->find("Sec-WebSocket-Accept");
            if(v == client->resqonse.HeadParameter->end())
            {
                evbuffer_free(evbuff);
                return -1;
            }
            evbuffer_add_printf(evbuff, "%s: %s\r\n", v->first.c_str(),v->second.c_str());
            client->resqonse.HeadParameter->erase(v);
            time_t now;
            struct tm *tm_now;
            char timeStr[50] = {0};
            time(&now);
            tm_now = localtime(&now);
            strftime(timeStr, sizeof(timeStr), "%a, %d %b %Y %T %Z", tm_now);
            evbuffer_add_printf(evbuff, "Date: %s\r\n", timeStr);
            evbuffer_add_printf(evbuff, "\r\n");
            client->stat = HTTP_WS_DATA;
            break;
        }
        else
        {
            switch(client->resqonse.resqonseCode)
            {
            case HTTP_NOCONTENT:
                evbuffer_add_printf(evbuff, "%s %d %s\r\n", client->request.http_version,
                                    HTTP_NOCONTENT, HTTP_NOCONTENT_VALUE);
                if(client->resqonse.resqonse_data == NULL)
                {
                    evbuffer_add_printf(evbuff, "Content-Type: text/html\r\n");
                    evbuffer_add_printf(evbuff, "Content-Length: %d\r\n", (int)strlen(HTTP_NOCONTENT_CONTENT));
                    evbuffer_add_printf(evbuff, "%s: %s\r\n", "Server", SERVER_STRING);
                    evbuffer_add_printf(evbuff, "\r\n");
                    evbuffer_add_printf(evbuff, HTTP_NOCONTENT_CONTENT);
                }
                break;
            case HTTP_MOVEPERM:
                evbuffer_add_printf(evbuff, "%s %d %s\r\n", client->request.http_version,
                                    HTTP_MOVEPERM, HTTP_MOVEPERM_VALUE);
                if(client->resqonse.resqonse_data == NULL)
                {
                    evbuffer_add_printf(evbuff, "Content-Type: text/html\r\n");
                    evbuffer_add_printf(evbuff, "Content-Length: %d\r\n", (int)strlen(HTTP_MOVEPERM_CONTENT));
                    evbuffer_add_printf(evbuff, "%s: %s\r\n", "Server", SERVER_STRING);
                    evbuffer_add_printf(evbuff, "\r\n");
                    evbuffer_add_printf(evbuff, HTTP_MOVEPERM_CONTENT);
                }
                break;
            case HTTP_MOVETEMP:
                evbuffer_add_printf(evbuff, "%s %d %s\r\n", client->request.http_version,
                                    HTTP_MOVETEMP, HTTP_MOVETEMP_VALUE);
                if(client->resqonse.resqonse_data == NULL)
                {
                    evbuffer_add_printf(evbuff, "Content-Type: text/html\r\n");
                    evbuffer_add_printf(evbuff, "Content-Length: %d\r\n", (int)strlen(HTTP_MOVETEMP_CONTENT));
                    evbuffer_add_printf(evbuff, "%s: %s\r\n", "Server", SERVER_STRING);
                    evbuffer_add_printf(evbuff, "\r\n");
                    evbuffer_add_printf(evbuff, HTTP_MOVETEMP_CONTENT);
                }
                break;
            case HTTP_NOTMODIFIED:
                evbuffer_add_printf(evbuff, "%s %d %s\r\n", client->request.http_version,
                                    HTTP_NOTMODIFIED, HTTP_NOTMODIFIED_VALUE);
                if(client->resqonse.resqonse_data == NULL)
                {
                    evbuffer_add_printf(evbuff, "%s: %s\r\n", "Server", SERVER_STRING);
                    evbuffer_add_printf(evbuff, "\r\n");
                }
                break;
            case HTTP_BADREQUEST:
                evbuffer_add_printf(evbuff, "%s %d %s\r\n", client->request.http_version,
                                    HTTP_BADREQUEST, HTTP_BADREQUEST_VALUE);
                if(client->resqonse.resqonse_data == NULL)
                {
                    evbuffer_add_printf(evbuff, "Content-Type: text/html\r\n");
                    evbuffer_add_printf(evbuff, "Content-Length: %d\r\n", (int)strlen(HTTP_BADREQUEST_CONTENT));
                    evbuffer_add_printf(evbuff, "%s: %s\r\n", "Server", SERVER_STRING);
                    evbuffer_add_printf(evbuff, "\r\n");
                    evbuffer_add_printf(evbuff, HTTP_BADREQUEST_CONTENT);
                }
                break;
            case HTTP_NOTFOUND:
                evbuffer_add_printf(evbuff, "%s %d %s\r\n", client->request.http_version,
                                    HTTP_NOTFOUND, HTTP_NOTFOUND_VALUE);
                if(client->resqonse.resqonse_data == NULL)
                {
                    evbuffer_add_printf(evbuff, "Content-Type: text/html\r\n");
                    evbuffer_add_printf(evbuff, "Content-Length: %d\r\n", (int)strlen(HTTP_NOTFOUND_CONTENT));
                    evbuffer_add_printf(evbuff, "%s: %s\r\n", "Server", SERVER_STRING);
                    evbuffer_add_printf(evbuff, "\r\n");
                    evbuffer_add_printf(evbuff, HTTP_NOTFOUND_CONTENT);
                }
                break;
            case HTTP_BADMETHOD:
                evbuffer_add_printf(evbuff, "%s %d %s\r\n", client->request.http_version,
                                    HTTP_BADMETHOD, HTTP_BADMETHOD_VALUE);
                if(client->resqonse.resqonse_data == NULL)
                {
                    evbuffer_add_printf(evbuff, "Content-Type: text/html\r\n");
                    evbuffer_add_printf(evbuff, "Content-Length: %d\r\n", (int)strlen(HTTP_BADMETHOD_CONTENT));
                    evbuffer_add_printf(evbuff, "%s: %s\r\n", "Server", SERVER_STRING);
                    evbuffer_add_printf(evbuff, "\r\n");
                    evbuffer_add_printf(evbuff, HTTP_BADMETHOD_CONTENT);
                }
                break;
            case HTTP_ENTITYTOOLARGE:
                evbuffer_add_printf(evbuff, "%s %d %s\r\n", client->request.http_version,
                                    HTTP_ENTITYTOOLARGE, HTTP_ENTITYTOOLARGE_VALUE);
                if(client->resqonse.resqonse_data == NULL)
                {
                    evbuffer_add_printf(evbuff, "Content-Type: text/html\r\n");
                    evbuffer_add_printf(evbuff, "Content-Length: %d\r\n", (int)strlen(HTTP_ENTITYTOOLARGE_CONTENT));
                    evbuffer_add_printf(evbuff, "%s: %s\r\n", "Server", SERVER_STRING);
                    evbuffer_add_printf(evbuff, "\r\n");
                    evbuffer_add_printf(evbuff, HTTP_ENTITYTOOLARGE_CONTENT);
                }
                break;
            case HTTP_EXPECTATIONFAILED:
                evbuffer_add_printf(evbuff, "%s %d %s\r\n", client->request.http_version,
                                    HTTP_EXPECTATIONFAILED, HTTP_EXPECTATIONFAILED_VALUE);
                if(client->resqonse.resqonse_data == NULL)
                {
                    evbuffer_add_printf(evbuff, "Content-Type: text/html\r\n");
                    evbuffer_add_printf(evbuff, "Content-Length: %d\r\n", (int)strlen(HTTP_EXPECTATIONFAILED_CONTENT));
                    evbuffer_add_printf(evbuff, "%s: %s\r\n", "Server", SERVER_STRING);
                    evbuffer_add_printf(evbuff, "\r\n");
                    evbuffer_add_printf(evbuff, HTTP_EXPECTATIONFAILED_CONTENT);
                }
                break;
            case HTTP_INTERNAL:
                evbuffer_add_printf(evbuff, "%s %d %s\r\n", client->request.http_version,
                                    HTTP_INTERNAL, HTTP_INTERNAL_VALUE);
                if(client->resqonse.resqonse_data == NULL)
                {
                    evbuffer_add_printf(evbuff, "Content-Type: text/html\r\n");
                    evbuffer_add_printf(evbuff, "Content-Length: %d\r\n", (int)strlen(HTTP_INTERNAL_CONTENT));
                    evbuffer_add_printf(evbuff, "%s: %s\r\n", "Server", SERVER_STRING);
                    evbuffer_add_printf(evbuff, "\r\n");
                    evbuffer_add_printf(evbuff, HTTP_INTERNAL_CONTENT);
                }
                break;
            case HTTP_NOTIMPLEMENTED:
                evbuffer_add_printf(evbuff, "%s %d %s\r\n", client->request.http_version,
                                    HTTP_NOTIMPLEMENTED, HTTP_NOTIMPLEMENTED_VALUE);
                if(client->resqonse.resqonse_data == NULL)
                {
                    evbuffer_add_printf(evbuff, "Content-Type: text/html\r\n");
                    evbuffer_add_printf(evbuff, "Content-Length: %d\r\n", (int)strlen(HTTP_NOTFOUND_CONTENT));
                    evbuffer_add_printf(evbuff, "%s: %s\r\n", "Server", SERVER_STRING);
                    evbuffer_add_printf(evbuff, "\r\n");
                    evbuffer_add_printf(evbuff, HTTP_NOTFOUND_CONTENT);
                }
                break;
            case HTTP_SERVUNAVAIL:
                evbuffer_add_printf(evbuff, "%s %d %s\r\n", client->request.http_version,
                                    HTTP_SERVUNAVAIL, HTTP_SERVUNAVAIL_VALUE);
                if(client->resqonse.resqonse_data == NULL)
                {
                    evbuffer_add_printf(evbuff, "Content-Type: text/html\r\n");
                    evbuffer_add_printf(evbuff, "Content-Length: %d\r\n", (int)strlen(HTTP_SERVUNAVAIL_CONTENT));
                    evbuffer_add_printf(evbuff, "%s: %s\r\n", "Server", SERVER_STRING);
                    evbuffer_add_printf(evbuff, "\r\n");
                    evbuffer_add_printf(evbuff, HTTP_SERVUNAVAIL_CONTENT);
                }
                break;
            }
            break;
        }
        if(client->resqonse.HeadParameter == NULL)
        {
            evbuffer_add_printf(evbuff, "%s: %s\r\n", "Server", SERVER_STRING);
        }
        else
        {
            v  =  client->resqonse.HeadParameter->find("Server");
            if(v != client->resqonse.HeadParameter->end())
            {
                evbuffer_add_printf(evbuff, "%s: %s\r\n", v->first.c_str(), v->second.c_str());
                client->resqonse.HeadParameter->erase(v);
            }
            else
            {
                evbuffer_add_printf(evbuff, "%s: %s\r\n", "Server", SERVER_STRING);
            }
            v = client->resqonse.HeadParameter->begin();
            while(v != client->resqonse.HeadParameter->end())
            {
                evbuffer_add_printf(evbuff, "%s: %s\r\n", v->first.c_str(), v->second.c_str());
                v++;
            }
        }
        evbuffer_add_printf(evbuff, "\r\n");

        if(client->resqonse.resqonse_data != NULL)
        {
            evbuffer_add(evbuff, client->resqonse.resqonse_data, client->resqonse.Resqonse_data_len);
        }
    }while(0);
    if(client->stat != HTTP_WS_DATA)
        client->stat = HTTP_RESQONSE;
    bufferevent_lock(client->bev);
    evbuffer_add_buffer(bufferevent_get_output(client->bev), evbuff);
    bufferevent_unlock(client->bev);

    pthread_mutex_unlock(&client->lock);
    evbuffer_free(evbuff);
    return 0;
}
/**********************************************************************/
/**********************************************************************/
/* A request has caused a call to accept() on the server port to
 * return.  Process the request appropriately.
 * Parameters: the socket connected to the client */
/**********************************************************************/
int accept_request(struct httpChilent_t * client)
{
    size_t numchars;
    char * buf;
    size_t i, j;
    struct evbuffer * evbuff = NULL;
    if(client == NULL)
        return REQUEST_ERROR;
    pthread_mutex_lock(&client->lock);
    gettimeofday(&client->final_optime, NULL);
    evbuff = client->receive_evbuff; 
    evbuffer_lock(client->receive_evbuff);
    buf = evbuffer_readln(evbuff, &numchars, EVBUFFER_EOL_CRLF);
    evbuffer_unlock(client->receive_evbuff);
    if(buf == NULL)
    {
        pthread_mutex_unlock(&client->lock);
        return REQUEST_NOCONTENT;
    }
    client->request.method = (char *)malloc(METHOD_MAX_LEN + 1);
    client->request.url = (char *)malloc(URL_MAX_LEN + 1);
    client->request.http_version = (char *)malloc(HTTPVERSION_MAX_LEN + 1);
    if(client->request.method == NULL || client->request.url == NULL || client->request.http_version == NULL)
    {
        pthread_mutex_unlock(&client->lock);
        return REQUEST_INTERNAL;
    }


    i = 0; j = 0;
    while (!ISspace(buf[j]) && (i < METHOD_MAX_LEN) && (j < numchars))
    {
        client->request.method[i] = buf[j];
        i++; j++;
    }
    if(j == numchars)
    {
        http_request_free(&client->request);
        pthread_mutex_unlock(&client->lock);
        return REQUEST_BADREQUEST;        
    }


    client->request.method[i] = '\0';

    if (strcasecmp(client->request.method, "GET") && strcasecmp(client->request.method, "POST"))
    {
        http_request_free(&client->request);
        pthread_mutex_unlock(&client->lock);
        return REQUEST_NOTIMPLEMENTED;
    }




    i = 0;
    while (ISspace(buf[j]) && (j < numchars) )
        j++;
    if(j == numchars)
    {
        http_request_free(&client->request);
        pthread_mutex_unlock(&client->lock);
        return REQUEST_BADREQUEST;        
    }
    while (!ISspace(buf[j]) && (i < URL_MAX_LEN) && (j < numchars))
    {
        client->request.url[i] = buf[j];
        i++; j++;
    }
    if(j == numchars)
    {
        http_request_free(&client->request);
        pthread_mutex_unlock(&client->lock);
        return REQUEST_BADREQUEST;        
    }



    client->request.url[i] = '\0';

    if (strcasecmp(client->request.method, "GET") == 0)
    {
        client->request.query_string = client->request.url;
        while ((*client->request.query_string != '?') && (*client->request.query_string != '\0'))
            client->request.query_string++;
        if (*client->request.query_string == '?')
        {
            *client->request.query_string = '\0';
            client->request.query_string++;
        }
    }
    else
    {
        if(strstr(client->request.url, "?"))
        {
            http_request_free(&client->request);
            pthread_mutex_unlock(&client->lock);
            return REQUEST_BADREQUEST;        
        }
    }



    i = 0;
    while (ISspace(buf[j]) && (j < numchars) )
        j++;
    if(j == numchars)
    {
        http_request_free(&client->request);
        pthread_mutex_unlock(&client->lock);
        return REQUEST_BADREQUEST;        
    }
    while (!ISspace(buf[j]) && (i < HTTPVERSION_MAX_LEN) && (j < numchars))
    {
        client->request.http_version[i] = buf[j];
        i++; j++;
    }
    if(strlen(client->request.http_version) < 8)
    {
        http_request_free(&client->request);
        pthread_mutex_unlock(&client->lock);
        return REQUEST_BADREQUEST;        
    }
    free(buf);
    while(1)
    {
        evbuffer_lock(client->receive_evbuff);
        buf = evbuffer_readln(evbuff, &numchars, EVBUFFER_EOL_CRLF);
        evbuffer_unlock(client->receive_evbuff);
        if(buf == NULL)
            break;
        else if(strcasecmp(client->request.method, "POST") &&  numchars == 0)
        {
            client->request.request_data_len = evbuffer_get_length(evbuff);
            client->request.request_data = (char *)malloc(client->request.request_data_len);
            if(client->request.request_data == NULL)
            {
                http_request_free(&client->request);
                pthread_mutex_unlock(&client->lock);
                return REQUEST_BADREQUEST;        
            }
            evbuffer_lock(client->receive_evbuff);
            evbuffer_remove(evbuff, client->request.request_data, client->request.request_data_len);
            evbuffer_unlock(client->receive_evbuff);
            break;
        }
        if( client->request.HeadParameter == NULL)
            client->request.HeadParameter = new HeadParameterMap();
        char * key = buf;
        char * v = strchr(buf,':');
        if(v == NULL)
        {
            http_request_free(&client->request);
            pthread_mutex_unlock(&client->lock);
            return REQUEST_BADREQUEST;        
        }
        *v = '\0';
        v++;
        i = 0;j = 0;
        while (ISspace(*key) && (j < numchars) )
        {
            j++;
            key++;
        }
        while (!ISspace(key[i]) && (i < strlen(key)))
        {
            i++; j++;
        }
        key[i] = '\0';
        i = 0;j = 0;
        numchars = v - buf;
        while (ISspace(*v) && (j < numchars) )
        {
            j++;
            v++;
        }
        while (!ISspace(v[i]) && (i < strlen(v)))
        {
            i++; j++;
        }
        v[i] = '\0';

        client->request.HeadParameter->insert(std::pair<STRING, STRING>(key, v));    
    }

    if(strcasecmp(client->request.method, "POST") && client->request.request_data == NULL)
    {
        http_request_free(&client->request);
        pthread_mutex_unlock(&client->lock);
        return REQUEST_BADREQUEST;        
    }
    pthread_mutex_unlock(&client->lock);

    return 0;

}


static void bufferev_write_cb(struct bufferevent *bev,void *ctx)
{
    bev = (struct bufferevent *)bev;
    httpChilent_t *client = (httpChilent_t *)ctx;
    pthread_mutex_lock(&client->lock);
    if( client->stat != HTTP_WS_DATA)
        client->stat = HTTP_IDLE;
    pthread_mutex_unlock(&client->lock);
}
void *httpd_handler_thread(void *arg)
{
    struct httpChilent_t *client = (struct httpChilent_t *)arg;
    struct httpServer_t *httpServer = client->httpServer;
    struct bufferevent * bev = client->bev;
    char respondKey[128];
    char c = 0;
    pthread_mutex_lock(&client->lock);
    gettimeofday(&client->final_optime, NULL);
    do
    {

        bufferevent_lock(bev);
        evbuffer_lock(client->receive_evbuff);
        bufferevent_disable(bev, EV_READ);
        while(1)
        {
            if(bufferevent_read(bev,&c,1) == 0 )
            {
                break;
            }
            evbuffer_add(client->receive_evbuff, &c , 1);
        }
        bufferevent_enable(bev, EV_READ);
        evbuffer_unlock(client->receive_evbuff);
        bufferevent_unlock(bev);


        pthread_mutex_unlock(&client->lock);
        int ret = accept_request(client);
        pthread_mutex_lock(&client->lock);
        if(ret != 0)
        {
            switch(ret)
            {
            case REQUEST_ERROR:
                client->resqonse.resqonseCode = HTTP_INTERNAL;
                break;
            case REQUEST_NOCONTENT:
                client->resqonse.resqonseCode = HTTP_NOCONTENT;
                break;
            case REQUEST_BADREQUEST:
                client->resqonse.resqonseCode = HTTP_BADREQUEST;
                break;
            case REQUEST_ENTITYTOOLARGE:
                client->resqonse.resqonseCode = HTTP_ENTITYTOOLARGE;
                break;
            case REQUEST_INTERNAL:
                client->resqonse.resqonseCode = HTTP_INTERNAL;
                break;
            case REQUEST_NOTIMPLEMENTED:
                client->resqonse.resqonseCode = HTTP_NOTIMPLEMENTED;
                break;
            default:
                client->resqonse.resqonseCode = HTTP_INTERNAL;
                break;
            }
            pthread_mutex_unlock(&client->lock);
            resqonse(client);
            pthread_mutex_lock(&client->lock);
            http_request_free(&client->request);
            http_request_free(&client->request);
            http_resqonse_free(&client->resqonse);
        }
        else
        {
            if(client->request.HeadParameter != NULL)
            {
                HeadParameterMap::iterator v = client->request.HeadParameter->find("Sec-WebSocket-Key");
                HeadParameterMap::iterator v1 = client->request.HeadParameter->find("Sec-WebSocket-Version");
                pthread_mutex_lock(&client->httpServer->lock);
                if(strcasecmp(client->request.method, "GET") == 0  && v1 != client->request.HeadParameter->end() && strcmp(v1->second.c_str(), "13") == 0  && v != client->request.HeadParameter->end() && strcmp(client->httpServer->webSocket_url, client->request.url) == 0)
                {
                    pthread_mutex_unlock(&client->httpServer->lock);
                    client->resqonse.resqonseCode = HTTP_SWITCHING_PROTOCOLS;
                    memset(respondKey, 0, sizeof(respondKey));
                    webSocket_buildRespondShakeKey((unsigned char *) v->second.c_str(), strlen(v->second.c_str()),( unsigned char *)respondKey);
                    client->resqonse.HeadParameter = new HeadParameterMap();
                    if(client->resqonse.HeadParameter == NULL)
                    {
                        client->resqonse.resqonseCode = HTTP_INTERNAL;
                        pthread_mutex_unlock(&client->lock);
                        resqonse(client);
                        pthread_mutex_lock(&client->lock);
                        http_request_free(&client->request);
                        http_resqonse_free(&client->resqonse);
                        break;
                    }
                    client->resqonse.HeadParameter->insert(HaedParameteMapPairi("Sec-WebSocket-Accept", respondKey));
                    pthread_mutex_unlock(&client->lock);
                    resqonse(client);
                    pthread_mutex_lock(&client->lock);
                    http_request_free(&client->request);
                    http_resqonse_free(&client->resqonse);
                    
                    break;
                }
                pthread_mutex_unlock(&client->httpServer->lock);
            }
            if(client->httpServer->handler != NULL)
            {
                httpd_handler_t handler;
                handler = client->httpServer->handler;
                client->resqonse.HeadParameter = new HeadParameterMap();
                pthread_mutex_unlock(&client->lock);
                handler(client);
                pthread_mutex_lock(&httpServer->clientMap_lock);
                BufferevMap::iterator v = httpServer->clientMap->find(bev);    
                if(httpServer->clientMap->end() == v)
                {
                    break;
                }
                pthread_mutex_unlock(&httpServer->clientMap_lock);
                pthread_mutex_lock(&client->lock);
                http_request_free(&client->request);
                http_resqonse_free(&client->resqonse);
            }
            else
            {
                //503错误
                client->resqonse.resqonseCode = HTTP_SERVUNAVAIL;
                pthread_mutex_unlock(&client->lock);
                resqonse(client);
                pthread_mutex_lock(&client->lock);
                http_request_free(&client->request);
                http_resqonse_free(&client->resqonse);

            }
        }
    }while(0);
    pthread_mutex_unlock(&client->lock);
    return NULL;

}

void *ws_read_cb_thread(void *arg)
{
    struct httpChilent_t *client = (struct httpChilent_t *)arg;
    struct httpServer_t *httpServer = client->httpServer;
    struct bufferevent * bev = client->bev;
    unsigned char * buff = NULL;
    unsigned int buff_len = 0;
    Websocket_CommunicationType ws_data_type;
    bufferevent_lock(client->bev);
    int ret = ev_webSocket_recv(client->bev, &buff, &buff_len, &ws_data_type);
    bufferevent_unlock(client->bev);
    if(ret <= 0)
    {
        if(ret == WCT_DISCONN)
        {
            httpChilent_Close(client);
            return NULL;
        }
    }
    else
    {
        client->ws_data_type = ws_data_type;
        if(client->stat == HTTP_WS_DATA && client->httpServer->ws_read_cb != NULL)
        {
            evbuffer_lock(client->receive_evbuff);
            if(evbuffer_add(client->receive_evbuff, buff, buff_len) != 0)
            {
                De_fprintf(stderr, "evbuffer_add fail!\r\n");
                evbuffer_unlock(client->receive_evbuff);
                pthread_mutex_unlock(&client->lock);
                free(buff);
                return NULL;
            }
            evbuffer_unlock(client->receive_evbuff);
            free(buff);
            webSocket_read_cb_t ws_read_cb = client->httpServer->ws_read_cb;
            pthread_mutex_unlock(&client->lock);
            ws_read_cb(client);
            pthread_mutex_lock(&httpServer->clientMap_lock);
            BufferevMap::iterator v = httpServer->clientMap->find(bev);    
            if(httpServer->clientMap->end() == v)
            {
                pthread_mutex_unlock(&httpServer->clientMap_lock);
                return NULL;
            }
            pthread_mutex_unlock(&httpServer->clientMap_lock);
            pthread_mutex_lock(&client->lock);
            client->ws_data_type = WCT_NULL;
        }
        else
            free(buff);
        pthread_mutex_unlock(&client->lock);
    }
    return NULL;
}
static void bufferev_read_cb(struct bufferevent *bev,void *ctx)
{
    bev = (struct bufferevent *)bev;
    struct httpChilent_t *client = (struct httpChilent_t *)ctx;
    pthread_mutex_lock(&client->lock);
    if(client->stat == HTTP_WS_DATA )
    {
        if(client->httpServer->ws_read_cb != NULL)
            threadpool_add(client->httpServer->pool, ws_read_cb_thread, client);
        pthread_mutex_unlock(&client->lock);
    }
    else
    {
        client->stat = HTTP_REQUEST;
        if(client->httpServer->handler != NULL)
            threadpool_add(client->httpServer->pool, httpd_handler_thread, client);
        pthread_mutex_unlock(&client->lock);
    }
}
void *http_supervise_thread(void * arg)
{
    httpServer_t *httpServer=(httpServer_t *)arg;
    pthread_mutex_lock(&httpServer->lock);
    while(!httpServer->shutdown)
    {
        pthread_mutex_unlock(&httpServer->lock);
        usleep(1000*HTTPSERVER_SUPERVISE_TIME);
        pthread_mutex_lock(&httpServer->lock);
        pthread_mutex_lock(&httpServer->clientMap_lock);
forstart:
        for(BufferevMap::iterator i = httpServer->clientMap->begin();
            i != httpServer->clientMap->end();i++)
        {
            struct timeval date;
            gettimeofday(&date, NULL);
            if((i->second)->stat != HTTP_WS_DATA && (date.tv_sec - (i->second)->final_optime.tv_sec) > HTTPSERVER_IDLE_TIMEROUT)
            {
                httpChilent_Close_nolock(i->second);
                goto forstart;
            }
        }
        pthread_mutex_unlock(&httpServer->clientMap_lock);
    }
    pthread_mutex_unlock(&httpServer->lock);
    return NULL;
}
int http_resqonse_free(struct http_resqonse_t *resqonse)
{
    if(resqonse == NULL)
        return -1;
    if(resqonse->resqonse_data != NULL)
        free(resqonse->resqonse_data);
    if(resqonse->HeadParameter != NULL)
        delete resqonse->HeadParameter;
    memset(resqonse, 0, sizeof(http_resqonse_t));
    return 0;
}
int http_request_free(struct http_request_t *request)
{
    if(request == NULL)
        return -1;
    if(request->method != NULL)
        free(request->method);
    if(request->request_data != NULL)
        free(request->request_data);
    if(request->url != NULL)
        free(request->url);
    if(request->http_version != NULL)
        free(request->http_version);
    if(request->HeadParameter != NULL)
        delete request->HeadParameter;
    memset(request, 0, sizeof(http_request_t));

    return 0;
}
int httpChilent_free(struct httpChilent_t* client)
{
    if(client == NULL)
        return-1;
    pthread_mutex_lock(&client->lock);
    if(client->receive_evbuff != NULL)
    {
        evbuffer_lock(client->receive_evbuff);
        evbuffer_free(client->receive_evbuff);
    }
    http_request_free(&client->request);
    http_resqonse_free(&client->resqonse);
    pthread_mutex_destroy(&client->lock);
    free(client);
    return 0;
}

static void bufferev_event_cb(struct bufferevent *bev, short events,void *ctx)
{
    httpChilent_t *client = (httpChilent_t *)ctx;
    struct sockaddr_in *address = NULL; 
    char ip[INET_ADDRSTRLEN];
    BufferevMap::iterator i;

    do
    {
        if(client == NULL)
        {
            De_printf("httpChilent NULL\r\n");
            break;
        }
        pthread_mutex_lock(&client->lock);
        address = (struct sockaddr_in *)&(client->addr);
        if(address != NULL)
        {
            inet_ntop(AF_INET, &address->sin_addr,ip, INET_ADDRSTRLEN);
        }
        if(events & BEV_EVENT_ERROR)
        {
            if(address != NULL)
                De_printf("%s:%d[ Error from bufferevnt:%s\r\n", inet_ntop(AF_INET,&address->sin_addr, ip,(socklen_t) sizeof(ip)), ntohs(address->sin_port), strerror(errno));  
            else
                De_printf("Unknown[ Error from bufferevnt:%s\r\n",strerror(errno));
        }
        else if(events & (BEV_EVENT_EOF))
        {
            if(address != NULL)
                De_printf("Connection close:%s:%d\r\n", inet_ntop(AF_INET,&address->sin_addr, ip,(socklen_t) sizeof(ip)), ntohs(address->sin_port));  
            else
                De_printf("Connection close: Unknown\r\n");
        }
        pthread_mutex_unlock(&client->lock);
    }
    while(0);
    bufferevent_lock(bev);
    bufferevent_disable(bev, EV_WRITE | EV_READ);
    if(client != NULL)
    {
        pthread_mutex_lock(&client->lock);

        evbase_threadpool_close_event(client->httpServer->evb_thpool, bufferevent_get_base(bev));
        pthread_mutex_lock(&client->httpServer->clientMap_lock);
        client->httpServer->clientMap->erase(bev);
        pthread_mutex_unlock(&client->httpServer->clientMap_lock);

        pthread_mutex_unlock(&client->lock);
        httpChilent_free(client);
    }
    close(bufferevent_getfd(bev));
    bufferevent_free(bev);
    /* bufferevent_unlock(bev); */
}
void httpChilent_Close_nolock(struct httpChilent_t * client)
{
    struct sockaddr_in *address = NULL; 
    char ip[INET_ADDRSTRLEN];
    BufferevMap::iterator i;

    do
    {
        if(client == NULL)
        {
            De_printf("httpChilent NULL\r\n");
            break;
        }
        address = (struct sockaddr_in *)&(client->addr);
        if(address != NULL)
        {
            inet_ntop(AF_INET, &address->sin_addr,ip, INET_ADDRSTRLEN);
        }
            if(address != NULL)
                De_printf("Connection close:%s:%d\r\n", inet_ntop(AF_INET,&address->sin_addr, ip,(socklen_t) sizeof(ip)), ntohs(address->sin_port));  
            else
                De_printf("Connection close: Unknown\r\n");
    }
    while(0);
    printf("连接关闭\r\n");
    bufferevent_lock(client->bev);
    bufferevent_disable(client->bev, EV_WRITE | EV_READ);
    if(client != NULL)
    {

        evbase_threadpool_close_event(client->httpServer->evb_thpool, bufferevent_get_base(client->bev));
        client->httpServer->clientMap->erase(client->bev);

        httpChilent_free(client);
    }
    close(bufferevent_getfd(client->bev));
    bufferevent_free(client->bev);

}
void httpChilent_Close(struct httpChilent_t * client)
{
    if(client == NULL)
        return;
    bufferev_event_cb(client->bev, BEV_EVENT_EOF, client);
}
static void listener_cd(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *arg)
{
    httpServer_t *httpServer=(httpServer_t *)arg;
    listener = (struct evconnlistener *)listener;
    struct httpChilent_t *client;
    char ip[INET_ADDRSTRLEN];
    struct sockaddr_in * addr = (struct sockaddr_in *)sa;
    inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

    event_base *base = evbase_threadpool_add_event(httpServer->evb_thpool);
    if(!base)
    {
        De_fprintf(stderr, "%s:%d[ base get fall!\r\n",ip, ntohs(addr->sin_port));
        return;
    }

    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if(bev == NULL)
    {
        De_fprintf(stderr, "%s:%d[ bufferevent new error:%s\r\n",ip, ntohs(addr->sin_port), evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        return;
    }


    client = (struct httpChilent_t *) malloc(sizeof(struct httpChilent_t));
    if(client == NULL)
    {
        De_fprintf(stderr, "%s:%d[ httpChilent malloc fail!\r\n",ip, ntohs(addr->sin_port));
        return;
    }
    memset(client, 0, sizeof(struct httpChilent_t));

    memcpy(&client->addr, sa, sizeof(sizeof(struct sockaddr_in)));

    client->bev = bev;
    client->httpServer = httpServer;

    if(gettimeofday(&client->final_optime, NULL) != 0)
    {
        De_fprintf(stderr, "%s:%d[ gettimeofday fail!\r\n",ip, ntohs(addr->sin_port));
        return;
    }
    client->stat = HTTP_CONNECTED;
    if(pthread_mutex_init(&client->lock, NULL)!=0)
    {
        De_fprintf(stderr, "client->lock init fail!\r\n");
        httpChilent_free(client);
        return;
    }

    client->receive_evbuff = evbuffer_new();
    if(client->receive_evbuff == NULL)
    {
        De_fprintf(stderr, "%s:%d[ evbuffer_new fail!\r\n",ip, ntohs(addr->sin_port));
        return;
    }
    evbuffer_enable_locking(client->receive_evbuff, NULL);

    bufferevent_setcb(bev, bufferev_read_cb, bufferev_write_cb, bufferev_event_cb, client);
    pthread_mutex_lock(&httpServer->clientMap_lock);
    httpServer->clientMap->insert(std::pair<struct bufferevent *, httpChilent_t *>(bev ,client));
    pthread_mutex_unlock(&httpServer->clientMap_lock);

    bufferevent_enable(bev, EV_READ| EV_WRITE);
    De_printf("client connection:%s:%d\r\n",ip, ntohs(addr->sin_port));
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

int httpServer_setHttpHandler(httpServer_t *httpServer, httpd_handler_t handler)
{
    if(httpServer == NULL)
        return -1;
    pthread_mutex_lock(&httpServer->lock);
    httpServer->handler = handler;
    pthread_mutex_unlock(&httpServer->lock);
    return 0;
}
int httpServer_setWebSocket_read_cb(httpServer_t * httpServer, webSocket_read_cb_t ws_read_cb,const char * url)
{
    if(httpServer == NULL)
        return -1;
    pthread_mutex_lock(&httpServer->lock);
    httpServer->webSocket_url = (char *)malloc(strlen(url)); 
    if(httpServer->webSocket_url == NULL)
    {
        return -1;
    }
    httpServer->ws_read_cb = ws_read_cb;
    strcpy(httpServer->webSocket_url, url);
    pthread_mutex_unlock(&httpServer->lock);
    return 0;
}
int httpServer_free(httpServer_t * httpServer)
{
    if(httpServer == NULL)
        return -1;
    httpServer->shutdown = true;
    if(httpServer->http_supervise_thread != 0)
        pthread_join(httpServer->http_supervise_thread, NULL);
    if(httpServer->http_listeber)
    {
        evconnlistener_free(httpServer->http_listeber);
    }
    if(httpServer->evb_thpool)
    {
        evbase_threadpool_destroy(httpServer->evb_thpool);
    }
    pthread_mutex_lock(&httpServer->clientMap_lock);
    if(httpServer->clientMap)
    {
        BufferevMap:: iterator i;
        for(i=httpServer->clientMap->begin();i != httpServer->clientMap->end(); i++)
        {
            httpChilent_free(i->second);
        }
        delete httpServer->clientMap;

    }
    pthread_mutex_destroy(&httpServer->lock);
    pthread_mutex_destroy(&httpServer->clientMap_lock);
    if(httpServer)
    {
        free(httpServer);
    }
    return 0;

}

httpServer_t * startHttpServe(struct event_base* base, u_short *port)
{
    httpServer_t * httpServer = NULL;
    struct sockaddr_in addr;
#ifdef DYNAMIC_CONFING
    if(SERVER_STRING_VARIALE == NULL)
        return NULL;
#endif
    do
    {
        if(base == NULL)
        {
            De_fprintf(stderr, "event_base == NULL\r\n");
            break;
        }
        httpServer = (httpServer_t *) malloc(sizeof(httpServer_t));
        if(httpServer == NULL)
        {
            De_fprintf(stderr, "httpServer_t mallocl fail!");
            break;
        }
        memset(httpServer, 0, (sizeof(httpServer_t)));

        httpServer->http_listener_base = base;
        httpServer->shutdown = false;

        httpServer->evb_thpool =  evbase_threadpool_new(HTTPSERVER_BASE_MAX_MONITOR_NUM, HTTPSERVER_MIN_THREAD_NUM);
        if(httpServer->evb_thpool == NULL)
        {
            De_fprintf(stderr, "new evbase_threadpool fail!\r\n");
            break;
        }
        if(pthread_mutex_init(&httpServer->lock, NULL) != 0)
        {
            De_fprintf(stderr, "httpServer->lock init fail\r\n");
            break;
        }
        if(pthread_mutex_init(&httpServer->clientMap_lock, NULL) != 0)
        {
            De_fprintf(stderr, "httpServer->clientMap_lock init fail\r\n");
            break;
        }
        httpServer->clientMap = new BufferevMap();
        if(httpServer->clientMap == NULL)
        {
            De_fprintf(stderr, "httpServer->clientMap new fail\r\n");
            break;    
        }
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(*port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if ((httpServer->http_listeber = evconnlistener_new_bind(base , listener_cd , httpServer, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,(struct sockaddr *)&addr, sizeof(addr))) == NULL)
        {
            De_fprintf(stderr, "evconnlistener_new_bind fail!\r\n");
            break;
        }
        httpServer->port = *port; 
        evconnlistener_set_error_cb(httpServer->http_listeber, accpt_error_cb);
        httpServer->pool = evbase_threadpool_get_threadpool(httpServer->evb_thpool);
        int ret = pthread_create(&httpServer->http_supervise_thread, NULL, http_supervise_thread, httpServer);
        if(ret != 0)
        {
            De_fprintf(stderr, "creatd thread fail:%s\r\n", strerror(ret));
            break;
        }
        return httpServer;
    }while(0);
    httpServer_free(httpServer);
    return NULL;
}
