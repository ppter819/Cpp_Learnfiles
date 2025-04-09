//防止头文件被重复包含
#pragma once
#include<sys/epoll.h>
#include<stdio.h>//NULL
#include<fcntl.h>
#include<errno.h> //EAGIN
#include<string.h> //strcasecmp strcmp
#include<unistd.h> //  chdir(argv[2]);
#include<sys/stat.h> //获取文件属性
#include <sys/socket.h>   // socket(), AF_INET, SOCK_STREAM
#include <netinet/in.h>   // sockaddr_in, IPPROTO_TCP
#include <arpa/inet.h>    // inet_addr(), htons()
#include<assert.h>
#include<sys/sendfile.h>
//初始化监听的套接字
int initListenFd(unsigned short port);
//启动epoll
int epollRun(int lfd);
//和客户端建立连接  监听
int acceptClient(int epfd,int lfd);
//接受http请求  通信
int recvHttpRequest(int epfd,int cfd);
//解析请求行  解析请求行以及回复数据  请求行为常量指针
int parseRequestLine(const char*line,int cfd);
//发送文件
int sendFile(const char* fileName, int cfd);

//发送响应头(状态行 响应头)
int sendHeadMsg(int cfd, int status, const char* descr, const char* contentType, int contentlen);
//根据文件后缀得到 得到对应的文件类型的content-type
const char* getFileType(const char* name);