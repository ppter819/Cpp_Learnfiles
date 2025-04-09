#include <stdio.h>

int main(int argc,char*argv[])
{

    if (argc<3)//参数个数小于3 那就输入if的语句 
        //等于3也就是有./a.out port path正好为3不报错
    {
        printf("./a.out port path\n");
        return -1;
    }
    //得到端口 传入的是字符串 字符串转化为int类型 隐式转化为给port
    unsigned short port = atoi(argv[1]);

    //切换服务器的工作路径 change dir
    chdir(argv[2]);//切换工作目录切换到指定资源的目录

    return 0;
    //初始化套接字
    //unsigned short 2字节短整型 0-65535
    int lfd=initListenFd(port);
    //启动程序  
    int epollRun(int lfd);
}