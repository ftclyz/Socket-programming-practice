#include "pch.h"
#include "IOmode.h"


IOmode::IOmode(int port, string ipaddr) {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	serveraddr.sin_addr.s_addr = inet_addr(ipaddr.c_str());
}

IOmode::~IOmode() {
	WSACleanup();
}

void IOmode::block_mode()
{
	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in clientaddr;
	unsigned long ul = 0;//0:阻塞，否则:非阻塞
	//控制socket I/O阻塞模式
	int nret = ioctlsocket(server, FIONBIO, (unsigned long*)&ul);
	//linux中对应设置为非阻塞
	//int flags = fcntl(server, F_GETFL, 0);        //获取文件的flags值。
	//fcntl(server, F_SETFL, flags | O_NONBLOCK);   //设置成非阻塞模式；

	bind(server, (sockaddr*)&serveraddr, sizeof(serveraddr));

	listen(server, 5);

	int len = sizeof(sockaddr);
	SOCKET client = accept(server, (sockaddr*)&clientaddr, &len);

	char buffer[1024];
	recv(server, buffer, 1023, 0);
}

void IOmode::select_mode()
{
	vector<SOCKET> clients;         //客户机套接字集合
	map<SOCKET, sockaddr_in> saddr; //套接字作为键值存储
	//为什么要这样声明，是因为假如有多台客户机连接到服务器，那么就是需要存储多台客户机的信息了


	fd_set readfd; //用于检查可读数据的套接字集合
	int ret = 0;

	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in clientaddr;
	unsigned long ul = 1;//0:阻塞，否则:非阻塞
	//控制socket I/O阻塞模式
	int nret = ioctlsocket(server, FIONBIO, (unsigned long*)&ul);

	bind(server, (sockaddr*)&serveraddr, sizeof(serveraddr));
	listen(server, 5);

	//不断的接收链接，并使用select检查socket是否可读
	while (true)
	{
		sockaddr_in addr;
		int len = sizeof(addr);
		int client = accept(server, (sockaddr*)&addr, &len);
		if (client != INVALID_SOCKET) {
			clients.push_back(client);
			saddr[client] = addr;
			//显示一下已连接的数量
			//inet_ntoa返回一个点分的十进制数串的指针，参数是sin_addr结构体类型
			cout << inet_ntoa(addr.sin_addr) << "已经连接..." << "当前连接数：" << clients.size() << endl;
		}

		//初始化readfd
		//必须初始化，否则会产生无法预料的后果
		FD_ZERO(&readfd);

		//检查所有连接进来的客户机
		for (int i = 0; i < clients.size(); i++) {
			FD_SET((int)clients[i], &readfd);
		}

		if (!clients.empty()) {
			timeval tv= { 0,0 };   //轮询
			ret = select(clients[clients.size() - 1] + 1, &readfd, NULL, NULL, &tv);
		}

		//if ret >0 也即有数据进来
		if (ret > 0) {
			vector<SOCKET> quit_client;
			for (int i = 0; i < clients.size(); i++) {
				if (FD_ISSET((int)clients[i], &readfd)) {   //检查每一个描述是否打开了
					char data[1024] = { 0 };
					recv(clients[i], data, 1023, 0);
					string recvdata = data;
					if (recvdata == "quit")
						quit_client.push_back(clients[i]);
					else
						cout << "来自" << inet_ntoa(saddr[clients[i]].sin_addr) << ":" << data << endl;
				}
			}

			//关闭退出的套接字
			if (!quit_client.empty()) {
				for (int i = 0; i < quit_client.size(); i++) {
					cout << "客户机" << inet_ntoa(saddr[quit_client[i]].sin_addr) << "已经退出，剩余连接数："
						<< quit_client.size() - 1 << endl;
					closesocket(quit_client[i]);
					vector<SOCKET>::iterator it = find(clients.begin(), clients.end(), quit_client[i]);
					clients.erase(it);
				}
			}
		}


	}
}

//pollfd结构体
//struct pollfd{
//       int fd;//文件描述符
//       short events;//所请求的事件
//      short revents;//实际返回的事件
//   }
/*
typedef std::vector<struct pollfd> PollFdList;
void IOmode::poll_mode()
{
	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(server, (sockaddr*)&serveraddr, sizeof(serveraddr));
	listen(server, 5);

	unsigned long ul = 1;//0:阻塞，否则:非阻塞
	//控制socket I/O阻塞模式
	int nret = ioctlsocket(server, FIONBIO, (unsigned long*)&ul);

	//poll
	struct pollfd pfd;
	pfd.fd = server;
	pfd.events = POLLIN;//关注pollin事件，表明有事件可读

	PollFdList pollfds;//创建一个动态数组（向量）
	pollfds.push_back(pfd);//把文件描述符添加到数组里面

	int nready;

	struct sockaddr_in clientaddr;
	socklen_t clientlen;
	int clientfd;

	//客户机信息
	int num = 0;  //当前连接数
	map<SOCKET, sockaddr_in> saddr;  //客户机地址信息

	while (true)
	{
		//tiomout为负值，将无限等待
		nready = WSAPoll(&(*pollfds.begin()), pollfds.size() - 1, -1);
		if (WSAGetLastError() == WSAEINVAL) cout << "erro";
		if (nready == 0)
			continue;

		clientfd = accept(server, (sockaddr*)&clientaddr, &clientlen);

		//把监听到的事件加入的队列中
		pfd.fd = clientfd;
		pfd.events = POLLIN;
		pfd.revents = 0;         //目前还没有任何事件返回，置为零
		pollfds.push_back(pfd);
		--nready;

		//保存一下客户机的信息
		saddr[clientfd] = clientaddr;
		num++;

		cout << "客户机" << inet_ntoa(clientaddr.sin_addr) << "已加入连接..."<< std::endl;

		if (nready == 0)//事件都处理完了
			continue;

		for (vector<pollfd>::iterator it = pollfds.begin() + 1; it != pollfds.end() && nready > 0; ++it)
		{
			if (it->revents & POLLIN)//如果是pollin事件
			{
				--nready;
				clientfd = it->fd;
				char buf[1024] = { 0 };
			
				recv(clientfd, buf, 1024, 0);

				if (buf == "quit") {
					num--;
					cout << "客户机" << inet_ntoa(saddr[clientfd].sin_addr) << "已断开.."
						<< "当前剩余连接数为：" << num;
					closesocket(clientfd);
				}

				std::cout << buf << endl;  //打印收到的消息内容
			}
		}
	}
}
*/