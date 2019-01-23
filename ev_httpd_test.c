/*************************************************************************
# > File Name: ev_httpd_test.c
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-01 14:05:47
# > Revise Time: 2018-10-29 13:17:00
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include <signal.h>
#include "include/ev_httpd.h"
#include "include/debug.h"
const char *getFileType(const char *filename){ //根据扩展名返回文件类型描述  
    char sExt[32];
    const char *p_start = filename + strlen(filename) - 1;
    memset(sExt, 0, sizeof(sExt));
    while (*p_start)
    {
        if (*p_start == '.')
        {
            p_start++;
            strncpy(sExt, p_start, sizeof(sExt));
            break;

        }
        p_start--;

    }

    if (strncmp(sExt, "bmp", 3) == 0)
        return "image/bmp";

    if (strncmp(sExt, "gif", 3) == 0)
        return "image/gif";

    if (strncmp(sExt, "ico", 3) == 0)
        return "image/x-icon";

    if (strncmp(sExt, "jpg", 3) == 0)
        return "image/jpeg";

    if (strncmp(sExt, "avi", 3) == 0)
        return "video/avi";

    if (strncmp(sExt, "css", 3) == 0)
        return "text/css";

    if (strncmp(sExt, "dll", 3) == 0)
        return "application/x-msdownload";

    if (strncmp(sExt, "exe", 3) == 0)
        return "application/x-msdownload";

    if (strncmp(sExt, "dtd", 3) == 0)
        return "text/xml";

    if (strncmp(sExt, "mp3", 3) == 0)
        return "audio/mp3";

    if (strncmp(sExt, "mpg", 3) == 0)
        return "video/mpg";

    if (strncmp(sExt, "png", 3) == 0)
    {
        return "image/png";
    }

    if (strncmp(sExt, "ppt", 3) == 0)
        return "application/vnd.ms-powerpoint";

    if (strncmp(sExt, "xls", 3) == 0)
        return "application/vnd.ms-excel";

    if (strncmp(sExt, "doc", 3) == 0)
        return "application/msword";

    if (strncmp(sExt, "mp4", 3) == 0)
        return "video/mpeg4";

    if (strncmp(sExt, "ppt", 3) == 0)
        return "application/x-ppt";

    if (strncmp(sExt, "wma", 3) == 0)
        return "audio/x-ms-wma";

    if (strncmp(sExt, "wmv", 3) == 0)
        return "video/x-ms-wmv";
    if(strncmp(sExt, "wmv", 3) == 0)
        return "application/x-javascript";
    return "text/html";
}
int getFileSize(const char * filename)
{
    int fileSize = 0;
    FILE *fp = NULL;
    if (filename == NULL)
        return 0;
    //以二进制的形式读取
    fp = fopen(filename, "rb");
    if (fp == NULL)
        return 0;
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    rewind(fp);
    fclose(fp);
    return fileSize;
}

void serve_file(struct httpChilent_t *  client, const char *filename)
{
    int fp = open(filename,O_RDONLY|O_NONBLOCK);
    char num[20];
    if (fp == -1)
    {
        client->resqonse.resqonseCode = HTTP_NOTFOUND;
    }
    else
    {
        int len = getFileSize(filename);
        if(len == 0)
        {
            client->resqonse.resqonseCode = HTTP_INTERNAL;
            close(fp);
            return;
        }
        sprintf(num,"%d",len);
        client->resqonse.Resqonse_data_len = len;
        client->resqonse.resqonse_data = (char *)malloc(client->resqonse.Resqonse_data_len);
        if(client->resqonse.resqonse_data == NULL)
        {
            client->resqonse.resqonseCode = HTTP_INTERNAL;
            close(fp);
            return;
        }
        client->resqonse.resqonseCode = HTTP_OK;
        client->resqonse.HeadParameter->insert(HaedParameteMapPairi("Content-Type", getFileType(filename)));
        client->resqonse.HeadParameter->insert(HaedParameteMapPairi("Content-Length",num));
        int rec;
        while((rec = read(fp, client->resqonse.resqonse_data, 1024)) >0)
        {
            pthread_mutex_lock(&client->lock);
            gettimeofday(&client->final_optime,NULL);
            pthread_mutex_unlock(&client->lock);
        }
        if(rec == -1)
        {
            perror("rev");
            free(client->resqonse.resqonse_data);
            client->resqonse.resqonse_data = NULL;
            client->resqonse.resqonseCode = HTTP_INTERNAL;
            close(fp);
            return;
        }
    }
    close(fp);
}
void httpd_handler(struct httpChilent_t * httpChilent) 
{
    char path[512];
    do{
        if(strcasecmp(httpChilent->request.method, "GET") == 0)
        {
            if(strcmp(httpChilent->request.url,"/") == 0 || strcmp(httpChilent->request.url, "/null") == 0)
            {
                sprintf(path, "./wwwroot/index.html");
            }
            else
            {
                sprintf(path, "./wwwroot%s", httpChilent->request.url);
                if(httpChilent->request.url[strlen(httpChilent->request.url) - 1] == '/')
                {
                    strcat(path, "index.html");
                }

            }
            serve_file(httpChilent, path);
            /*
            httpChilent->resqonse.resqonseCode = HTTP_OK;
            int len  = httpChilent->resqonse.Resqonse_data_len= strlen(HTTP_MOVEPERM_CONTENT);
            sprintf(num,"%d",len);
            httpChilent->resqonse.HeadParameter->insert(HaedParameteMapPairi("Content-Type", "text/html"));
            httpChilent->resqonse.HeadParameter->insert(HaedParameteMapPairi("Content-Length",num));
            httpChilent->resqonse.resqonse_data = (char *)malloc(httpChilent->resqonse.Resqonse_data_len);
            strcpy(httpChilent->resqonse.resqonse_data, HTTP_MOVEPERM_CONTENT);
            */
        }
        else if(strcasecmp(httpChilent->request.method, "POST") == 0)
        {
             
        }
        if(httpChilent->request.HeadParameter != NULL)
        {
            HeadParameterMap::iterator v;
            v = httpChilent->request.HeadParameter->find("Connection");
            if(v != httpChilent->request.HeadParameter->end())
            {
                if(strcasecmp(v->second.c_str(), "keep-alive") == 0)
                {

                }
                else if(strcasecmp(v->second.c_str(), "close") == 0)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        resqonse(httpChilent);
        return;
    }while(0);
    resqonse(httpChilent);
    httpChilent_Close(httpChilent);
}
void ws_read_cb(struct httpChilent_t * httpChilent)
{
    char buff[1024];
    char ip[INET_ADDRSTRLEN];
    struct sockaddr_in *address = (struct sockaddr_in *)&(httpChilent->addr);
    inet_ntop(AF_INET, &address->sin_addr,ip, INET_ADDRSTRLEN);
    evbuffer_remove(httpChilent->receive_evbuff, buff, evbuffer_get_length(httpChilent->receive_evbuff));
    printf("%s:%d ] %s\r\n",ip, ntohs(address->sin_port) ,  buff);
    ev_webSocket_send(httpChilent->bev, (unsigned char *)"hello!", (unsigned)6,false,WCT_TXTDATA);
}
void close_signal_cb(evutil_socket_t sig,short events, void *user_data)
{
    struct event_base *base = (struct event_base *)user_data;
    struct timeval delay = {2, 0};
    if(sig == SIGINT)
    {
        De_printf("2s后程序关闭\r\n");
        event_base_loopexit(base, &delay);
    }
}
int main(void)
{
    u_short port = 8088;
    struct event_base *base;
    struct event * close_signal; 
    httpServer_t * httpServer;
    base = event_base_new();
    set_SERVER_STRING(NULL);
    httpServer = startHttpServe(base ,&port);
    httpServer_setHttpHandler(httpServer, httpd_handler);
    httpServer_setWebSocket_read_cb(httpServer, ws_read_cb, "/");
    close_signal =  evsignal_new(base,SIGINT,close_signal_cb, base); 
    if(!close_signal || event_add(close_signal, NULL) < 0)
    {
        De_fprintf(stderr, "Could not create/add a signal event!\n");
        exit(1);
    }
    event_base_dispatch(base);
    httpServer_free(httpServer);

    event_base_free(base);

    return(0);
}
