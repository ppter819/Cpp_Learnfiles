#include"server.h"



int initListenFd(unsigned short port)
{	//1.�����׽���
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd==-1)
	{
		perror("socket wrong");
		return -1;
	} 
	//2.�˿ڸ��� setsocket���� int setsockopt
	//Ĭ��ָ�������Ͽ����ӵ�һ�� һ����֮��ſ���ֱ�Ӷ˿�
	//������ƶ˿ڸ��� ����ֱ��ʹ��
	//��ô���ÿ���ֱ�ӷ��ʶ˿�
	int opt = 1;
	int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret==-1)
		{
			perror("socketopt worng");
			return -1;
	}
	//3.��bind
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET; 
	saddr.sin_port = htons(port); //�������port��������  host to net 
	saddr.sin_addr.s_addr = INADDR_ANY;//0��ַ
	 ret = bind(lfd,(struct sockaddr*)&saddr,sizeof(saddr) );
	if (ret == -1)
	{
		perror("bind wrong");
		return -1;
	}
	//4���ü���
	
	ret = listen(lfd, 128);//�������128 
	if (ret==-1)
	{
		perror("listen wrong");
		return -1;
	}
	return lfd;
}



int epollRun(int lfd)
{
	//1.����epollʵ��
	int epfd = epoll_create(1);//����0���� 1��ʵ������
	if (epfd==-1)
	{
		perror("epoll worng");
		return -1;
	}
	//2.lfd���� ��������
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;//������ ˮƽ����
	ev.data.fd = lfd;
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);//������Ϣ
	if (ret==-1)
	{

		perror("���ʧ��");
		return -1;
	}
	//3.����һ��epoll_event������ ���ڴ��׼���õ�������
	struct epoll_event  evs[1024];
	//4.��� ѭ������
	while (1)//��true��Ҫͷ�ļ�
	{
		//�Ƽ�д-1����
		//�˴�Ӧ�õõ�Ԫ�ظ��� sizeof(evs)/sizeof(evs[0])
		int num = epoll_wait(epfd, evs, sizeof(evs)/sizeof(evs[0]), -1);//-1����������
		for (int i = 0; i < num; i++)
		{
			int fd = evs[i].data.fd;//�Ѿ�׼���õ��ļ���������

			if (fd==lfd)
			{
				//a���Խ���ͨ�� ����accept����
				int acceptClient(epfd,  fd);//�˴�fd==lfd		
			}
			else 
			{
				//b ������ͨ�ŵ������� cfdִ�в���
				//������ν��ж�����
				recvHttpRequest(epfd,fd);//�˴�fd==cfd
			}
		
		}

	}

	
	return 0;
}

int acceptClient(int epfd, int lfd)
{
	//1.�������� 
	int cfd = accept(lfd, NULL, NULL);
	if (cfd == -1)
	{
		perror("accept worng"); return -1;
	}

	//2.����Ϊ���ط�����ģʽ ����cfd�õ��µ�cfd 
	//�˴�����������
	int flag = fcntl(cfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(cfd, F_SETFL, flag);

	//3.cfd��ӵ�epollģ��֮��
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;//���ش���ģʽ�� ������ģʽ
	ev.data.fd = cfd;
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);

	return 0;
}

int recvHttpRequest(int epfd,int cfd)
{
	int total = 0;   //totalͳ�Ƶ�ǰ���ݳ���
	char buffer[4096] = {0}; //ȫ������
	char temp[1024] = {0};//��ʱ����
	//�������� ����temp��
	int len = recv(cfd,temp,sizeof(temp),0);
	//��Ϊ�Ǳ���ģʽ������ ֻ��ִ��һ�� ��˱���ÿ�μ�������ȫ��ȡ����
	while (len>0) //ִ��ѭ�����ж�ȡ
	{
		//����ǰ���ݳ���+����temp�ĳ���<buffer������û������
		if (total+len<sizeof(buffer))
		{
			//temp���ݴ���buffer   
			//tempָ���len���ֽڴ���bufferָ����ڴ�����
			//memcpy(buffer, temp, len);

			//tempָ���len���ֽڴ���bufferƫ��λ�ô�
			//�ȼ���buffer[len]����ʼ����
			memcpy(buffer+total, temp, len);
			
		}
		total += len;//��ǰ���Ƚ�������
	
		//��Ϊ�Ǳ���ģʽ�ķ�����ģʽ �����Ҫ�ж��Ƿ��ȡ���
		//�˴���������� һ�����ݶ�ȡʧ�� һ�ֶ�ȡ��� ��ʹ��len==-1
		//���ݷ��ص�ֵ�Ĳ�ͬ��������
		if (len==-1 &&errno==EAGAIN)//��ȡ��� 
		{
			printf("���ݽ������");
			
			
			//���������� ��˴�ʱ�൱����������
			
		}
		else if(len==0)
		{
			//��ͻ��˶Ͽ������� ����ͨ������������ɾ��
			//�˴�����Ҫ�����������¼�������дNULL
			epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
			close(cfd);
		}

		else
		{
			perror("recv�д��� Ҳ�����ļ����ݽ���ʧ��");
		}
	}
	return 0;
}



//----------------------------------------------
//get /xxx/1.jpg http/1.1
int parseRequestLine(const char* line, int cfd)//const char* line���ַ���
{ 
	//���������� ֻдget
	//�Ƚ��б��� ���ÿո���м������ 
	char method[12]; //�����get or post ����Ҫ̫��
	char path[1024]; //����ͻ�������ľ�̬��Դ �ļ�·��
	sscanf(line, "%[^ ] %[^ ]", method, path);
	//�ж�Ϊget or post����
	//�����ַ������бȽ� ������ȷ���0 ����ȷ��ط�0
	if (strcasecmp(method,"get")!=0) //�����ִ�Сд 
	{
		return -1;
	}

	char* file = NULL;
	//�ж�һ���Ƿ� Ϊ��Ŀ¼ or Ŀ¼�ڵ��ļ�
	if (strcmp(path,"/")==0)//�ж��Ƿ���/��ô�ͽ�������
	{
		//�˴�����Ϊ  ./xxx/1.jpg���·��
		file = "./";
	}
	else
	{
		//�˴�����Ϊ  xxx/1.jpg   
		file = path + 1;//�൱��ȥ��б��
	}
	
	//��ȡ�ļ�����
	struct stat st;
	int ret = stat(file, &st);//�ж��ļ�״̬
	if (ret==-1)
	{
		//�ļ������� --����404  �ȷ���http��Ӧ �ٸ�����Ӧ�����ݿ�
		 sendHeadMsg(cfd,404,"NOT FOUND", getFileType(".html"), -1);//�˴�ָ��-1 �Լ���ȡ�ļ�����
		 sendFile("404.html", cfd);
		 return 0;
	}

	//�����ļ����� ��ô����
	//�ж��ļ����� ��Ŀ¼�ͷ�1 
	if (S_ISDIR(st.st_mode))
	{
		//��Ŀ¼���ݷ��Ϳͻ���
	}
	else
	{
		//��Ŀ¼�ڵ��ļ����ݷ��Ϳͻ���
		sendHeadMsg(cfd, 200, "OK", getFileType(file), st.st_size);//st.st_size�к����ļ���С
		sendFile(file, cfd);

	}
	return 0;
}




//�����ļ���
const char* getFileType(const char* name)
{
	// a.jpg a.mp4 a.html
	// ����������ҡ�.���ַ�, �粻���ڷ���NULL
	const char* dot = strrchr(name, '.');//���Ҳ�������ԡ�.������ʼ��ȡ
	//���Ҳ��һ���ҵ��ĵ�һ�� . һ�����ļ��ĺ�׺��
	if (dot == NULL)
		return "text/plain; charset=utf-8";	// ���ı�
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

	return "text/plain; charset=utf-8";//���ص��Ǵ��ı�
}





int sendFile(const char* fileName, int cfd)
{
	//1.���ļ� ֻ��
	int fd = open(fileName, O_RDONLY);
	assert(fd>0);//����0���� �����жϳ���

	//���ú����ע��
#if 0
	while (1)
	{
		char buffer[1024];
		int len = read(fd, buffer, sizeof(buffer));
		if (len>0)
		{
			send(cfd, buffer, len, 0);
			//sleep(10) usleep(10)ǰ�ߵ�λs ����΢s
			//˯�߸�����˽������� һЩʱ�� ���Ͷ˷�����һЩ
			usleep(10);
		}
		else if (len == 0)
		{
			break;//���ݶ���
		}
		else
		{
			perror("read wrong");
		}
	}
#else
	//��ȡ�ļ�ƫ����  �� 0 ��SEEK_END
	int size = lseek(fd, 0, SEEK_END);
	//���ļ����ݷ��͸��ͻ���
	sendfile(cfd, fd, NULL, size); //����send ������sendfile
#endif

	

	return 0;
}

//�ڵ���sendfile֮ǰ���øú���
int sendHeadMsg(int cfd, int status, const char* descr, const char* contentType, int contentLen)
{

	//����һ���ַ��������ڴ��HTTP��Ӧ
	char buffer[4096] = { 0 };
	//1.״̬��
	//status ״̬�� descr ״̬������ \r\n��http�б�ʾ����
	sprintf(buffer, "http/1.1 %d %s\r\n", status, descr);
	//2.��Ӧͷ
	//�Ѿ�����ĳ��� ��ʱ���ٽ��з���buffer==buffer[strlen(buffer)]
	sprintf(buffer + strlen(buffer), "content-type: %s\r\n", contentType);
	sprintf(buffer + strlen(buffer), "content-length: %d\r\n", contentLen);
	//3.����
	//4.��������
	send(cfd, buffer, strlen(buffer), 0);
	return 0;
}


