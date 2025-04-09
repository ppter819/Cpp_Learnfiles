#include"server.h"



int initListenFd(unsigned short port)
{	//1.监听套接字
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd==-1)
	{
		perror("socket wrong");
		return -1;
	} 
	//2.端口复用 setsocket函数 int setsockopt
	//默认指定主动断开连接的一方 一分钟之后才可以直接端口
	//现在设计端口复用 可以直接使用
	//这么设置可以直接访问端口
	int opt = 1;
	int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret==-1)
		{
			perror("socketopt worng");
			return -1;
	}
	//3.绑定bind
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET; 
	saddr.sin_port = htons(port); //将传入的port进行设置  host to net 
	saddr.sin_addr.s_addr = INADDR_ANY;//0地址
	 ret = bind(lfd,(struct sockaddr*)&saddr,sizeof(saddr) );
	if (ret == -1)
	{
		perror("bind wrong");
		return -1;
	}
	//4设置监听
	
	ret = listen(lfd, 128);//最多设置128 
	if (ret==-1)
	{
		perror("listen wrong");
		return -1;
	}
	return lfd;
}



int epollRun(int lfd)
{
	//1.创建epoll实例
	int epfd = epoll_create(1);//大于0即可 1无实际意义
	if (epfd==-1)
	{
		perror("epoll worng");
		return -1;
	}
	//2.lfd上树 三件事情
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;//读操作 水平触发
	ev.data.fd = lfd;
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);//传入信息
	if (ret==-1)
	{

		perror("添加失败");
		return -1;
	}
	//3.定义一个epoll_event的数组 用于存放准备好的描述符
	struct epoll_event  evs[1024];
	//4.检测 循环动作
	while (1)//用true需要头文件
	{
		//推荐写-1即可
		//此处应该得到元素个数 sizeof(evs)/sizeof(evs[0])
		int num = epoll_wait(epfd, evs, sizeof(evs)/sizeof(evs[0]), -1);//-1是设置阻塞
		for (int i = 0; i < num; i++)
		{
			int fd = evs[i].data.fd;//已经准备好的文件描述符号

			if (fd==lfd)
			{
				//a可以进行通信 调用accept函数
				int acceptClient(epfd,  fd);//此处fd==lfd		
			}
			else 
			{
				//b 被用于通信的描述符 cfd执行操作
				//处理如何进行读数据
				recvHttpRequest(epfd,fd);//此处fd==cfd
			}
		
		}

	}

	
	return 0;
}

int acceptClient(int epfd, int lfd)
{
	//1.建立连接 
	int cfd = accept(lfd, NULL, NULL);
	if (cfd == -1)
	{
		perror("accept worng"); return -1;
	}

	//2.设置为边沿非阻塞模式 传入cfd得到新的cfd 
	//此处可以设计如此
	int flag = fcntl(cfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(cfd, F_SETFL, flag);

	//3.cfd添加到epoll模型之中
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;//边沿触发模式的 非阻塞模式
	ev.data.fd = cfd;
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);

	return 0;
}

int recvHttpRequest(int epfd,int cfd)
{
	int total = 0;   //total统计当前数据长度
	char buffer[4096] = {0}; //全部数据
	char temp[1024] = {0};//临时数据
	//接收数据 放入temp中
	int len = recv(cfd,temp,sizeof(temp),0);
	//因为是边沿模式非阻塞 只会执行一次 因此必须每次检测把数据全部取出来
	while (len>0) //执行循环进行读取
	{
		//若当前数据长度+存入temp的长度<buffer容器（没填满）
		if (total+len<sizeof(buffer))
		{
			//temp数据存入buffer   
			//temp指向的len个字节存入buffer指向的内存区域
			//memcpy(buffer, temp, len);

			//temp指向的len个字节存入buffer偏移位置处
			//等价于buffer[len]处开始存入
			memcpy(buffer+total, temp, len);
			
		}
		total += len;//当前长度进行增加
	
		//因为是边沿模式的非阻塞模式 因此需要判断是否读取完毕
		//此处分两种情况 一种数据读取失败 一种读取完毕 均使得len==-1
		//根据返回的值的不同加以区分
		if (len==-1 &&errno==EAGAIN)//读取完毕 
		{
			printf("数据接收完毕");
			
			
			//解析请求行 因此此时相当于请求数据
			
		}
		else if(len==0)
		{
			//与客户端断开了连接 将该通信描述符进行删除
			//此处不需要对描述符的事件进行书写NULL
			epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
			close(cfd);
		}

		else
		{
			perror("recv有错误 也就是文件数据接受失败");
		}
	}
	return 0;
}



//----------------------------------------------
//get /xxx/1.jpg http/1.1
int parseRequestLine(const char* line, int cfd)//const char* line是字符串
{ 
	//解析请求行 只写get
	//先进行遍历 利用空格进行间隔开来 
	char method[12]; //存入的get or post 不需要太大
	char path[1024]; //存入客户端请求的静态资源 文件路径
	sscanf(line, "%[^ ] %[^ ]", method, path);
	//判断为get or post请求
	//利用字符串进行比较 两者相等返回0 不相等返回非0
	if (strcasecmp(method,"get")!=0) //不区分大小写 
	{
		return -1;
	}

	char* file = NULL;
	//判断一下是否 为根目录 or 目录内的文件
	if (strcmp(path,"/")==0)//判断是否有/那么就进行区分
	{
		//此处设置为  ./xxx/1.jpg相对路径
		file = "./";
	}
	else
	{
		//此处设置为  xxx/1.jpg   
		file = path + 1;//相当于去掉斜杠
	}
	
	//获取文件属性
	struct stat st;
	int ret = stat(file, &st);//判断文件状态
	if (ret==-1)
	{
		//文件不存在 --返回404  先发送http相应 再给出对应的数据快
		 sendHeadMsg(cfd,404,"NOT FOUND", getFileType(".html"), -1);//此处指定-1 自己读取文件长度
		 sendFile("404.html", cfd);
		 return 0;
	}

	//现在文件存在 那么接着
	//判断文件类型 是目录就返1 
	if (S_ISDIR(st.st_mode))
	{
		//将目录内容发送客户端
	}
	else
	{
		//将目录内的文件内容发送客户端
		sendHeadMsg(cfd, 200, "OK", getFileType(file), st.st_size);//st.st_size中含有文件大小
		sendFile(file, cfd);

	}
	return 0;
}




//传入文件名
const char* getFileType(const char* name)
{
	// a.jpg a.mp4 a.html
	// 自右向左查找‘.’字符, 如不存在返回NULL
	const char* dot = strrchr(name, '.');//从右侧向左侧以“.”处开始读取
	//从右侧第一个找到的第一个 . 一定是文件的后缀名
	if (dot == NULL)
		return "text/plain; charset=utf-8";	// 纯文本
	if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
		return "text/html; charset=utf-8";
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcmp(dot, ".gif") == 0)
		return "image/gif";
	if (strcmp(dot, ".png") == 0)
		return "image/png";
	if (strcmp(dot, ".css") == 0)
		return "text/css";
	if (strcmp(dot, ".au") == 0)
		return "audio/basic";
	if (strcmp(dot, ".wav") == 0)
		return "audio/wav";
	if (strcmp(dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
		return "video/quicktime";
	if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
		return "video/mpeg";
	if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
		return "model/vrml";
	if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
		return "audio/midi";
	if (strcmp(dot, ".mp3") == 0)
		return "audio/mpeg";
	if (strcmp(dot, ".ogg") == 0)
		return "application/ogg";
	if (strcmp(dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";

	return "text/plain; charset=utf-8";//返回的是纯文本
}





int sendFile(const char* fileName, int cfd)
{
	//1.打开文件 只读
	int fd = open(fileName, O_RDONLY);
	assert(fd>0);//大于0无事 否则中断程序

	//利用宏进行注释
#if 0
	while (1)
	{
		char buffer[1024];
		int len = read(fd, buffer, sizeof(buffer));
		if (len>0)
		{
			send(cfd, buffer, len, 0);
			//sleep(10) usleep(10)前者单位s 后者微s
			//睡眠给服务端接受数据 一些时间 发送端发送慢一些
			usleep(10);
		}
		else if (len == 0)
		{
			break;//数据读完
		}
		else
		{
			perror("read wrong");
		}
	}
#else
	//求取文件偏移量  从 0 到SEEK_END
	int size = lseek(fd, 0, SEEK_END);
	//将文件内容发送给客户端
	sendfile(cfd, fd, NULL, size); //不用send 而是用sendfile
#endif

	

	return 0;
}

//在调用sendfile之前调用该函数
int sendHeadMsg(int cfd, int status, const char* descr, const char* contentType, int contentLen)
{

	//定义一个字符数组用于存放HTTP相应
	char buffer[4096] = { 0 };
	//1.状态行
	//status 状态码 descr 状态描述符 \r\n再http中表示换行
	sprintf(buffer, "http/1.1 %d %s\r\n", status, descr);
	//2.响应头
	//已经放入的长度 这时候再进行放入buffer==buffer[strlen(buffer)]
	sprintf(buffer + strlen(buffer), "content-type: %s\r\n", contentType);
	sprintf(buffer + strlen(buffer), "content-length: %d\r\n", contentLen);
	//3.空行
	//4.发送数据
	send(cfd, buffer, strlen(buffer), 0);
	return 0;
}


