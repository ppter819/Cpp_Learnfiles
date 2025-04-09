//��ֹͷ�ļ����ظ�����
#pragma once
#include<sys/epoll.h>
#include<stdio.h>//NULL
#include<fcntl.h>
#include<errno.h> //EAGIN
#include<string.h> //strcasecmp strcmp
#include<unistd.h> //  chdir(argv[2]);
#include<sys/stat.h> //��ȡ�ļ�����
#include <sys/socket.h>   // socket(), AF_INET, SOCK_STREAM
#include <netinet/in.h>   // sockaddr_in, IPPROTO_TCP
#include <arpa/inet.h>    // inet_addr(), htons()
#include<assert.h>
#include<sys/sendfile.h>
//��ʼ���������׽���
int initListenFd(unsigned short port);
//����epoll
int epollRun(int lfd);
//�Ϳͻ��˽�������  ����
int acceptClient(int epfd,int lfd);
//����http����  ͨ��
int recvHttpRequest(int epfd,int cfd);
//����������  �����������Լ��ظ�����  ������Ϊ����ָ��
int parseRequestLine(const char*line,int cfd);
//�����ļ�
int sendFile(const char* fileName, int cfd);

//������Ӧͷ(״̬�� ��Ӧͷ)
int sendHeadMsg(int cfd, int status, const char* descr, const char* contentType, int contentlen);
//�����ļ���׺�õ� �õ���Ӧ���ļ����͵�content-type
const char* getFileType(const char* name);