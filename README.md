# 基于Libevent库与线程池编写的简单WebSocket服务器与客服端 #
>## 作者：SSZL 博客：[https://sszlbg.cn](https://sszlbg.cn)
>> * 编译需要安装 Libevent库 与 OpenSsl 库
>>> * Libevent安装方法
>>>> * yum install libevent-devel 
>>> * OpenSsl库安装
>>>> * yum install openssl
>>>> * yum install openssl-devel
>> * MakeFile文件说明
>>> * app:  ev_httpd_server_process ev_client_process HTTP-Websocket服务器（线程池Libevent库）与Websocket客户端
>>> * app:  ev_thread_server_process ev_client_process WebSocket服务器（线程池Libevent库）与WebSocket客户端
>>> * app:  ev_server_process ev_client_process WebSocket服务器（Libevent库）与WebSocket客户端
>>> * app: client_process server_process WebSocket服务器（epoll）与WebSocket客户端
>>> * 监听应用对应源码请 查看 MakeFile 文件
