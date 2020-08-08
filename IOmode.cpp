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
	unsigned long ul = 0;//0:����������:������
	//����socket I/O����ģʽ
	int nret = ioctlsocket(server, FIONBIO, (unsigned long*)&ul);
	//linux�ж�Ӧ����Ϊ������
	//int flags = fcntl(server, F_GETFL, 0);        //��ȡ�ļ���flagsֵ��
	//fcntl(server, F_SETFL, flags | O_NONBLOCK);   //���óɷ�����ģʽ��

	bind(server, (sockaddr*)&serveraddr, sizeof(serveraddr));

	listen(server, 5);

	int len = sizeof(sockaddr);
	SOCKET client = accept(server, (sockaddr*)&clientaddr, &len);

	char buffer[1024];
	recv(server, buffer, 1023, 0);
}

void IOmode::select_mode()
{
	vector<SOCKET> clients;         //�ͻ����׽��ּ���
	map<SOCKET, sockaddr_in> saddr; //�׽�����Ϊ��ֵ�洢
	//ΪʲôҪ��������������Ϊ�����ж�̨�ͻ������ӵ�����������ô������Ҫ�洢��̨�ͻ�������Ϣ��


	fd_set readfd; //���ڼ��ɶ����ݵ��׽��ּ���
	int ret = 0;

	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in clientaddr;
	unsigned long ul = 1;//0:����������:������
	//����socket I/O����ģʽ
	int nret = ioctlsocket(server, FIONBIO, (unsigned long*)&ul);

	bind(server, (sockaddr*)&serveraddr, sizeof(serveraddr));
	listen(server, 5);

	//���ϵĽ������ӣ���ʹ��select���socket�Ƿ�ɶ�
	while (true)
	{
		sockaddr_in addr;
		int len = sizeof(addr);
		int client = accept(server, (sockaddr*)&addr, &len);
		if (client != INVALID_SOCKET) {
			clients.push_back(client);
			saddr[client] = addr;
			//��ʾһ�������ӵ�����
			//inet_ntoa����һ����ֵ�ʮ����������ָ�룬������sin_addr�ṹ������
			cout << inet_ntoa(addr.sin_addr) << "�Ѿ�����..." << "��ǰ��������" << clients.size() << endl;
		}

		//��ʼ��readfd
		//�����ʼ�������������޷�Ԥ�ϵĺ��
		FD_ZERO(&readfd);

		//����������ӽ����Ŀͻ���
		for (int i = 0; i < clients.size(); i++) {
			FD_SET((int)clients[i], &readfd);
		}

		if (!clients.empty()) {
			timeval tv= { 0,0 };   //��ѯ
			ret = select(clients[clients.size() - 1] + 1, &readfd, NULL, NULL, &tv);
		}

		//if ret >0 Ҳ�������ݽ���
		if (ret > 0) {
			vector<SOCKET> quit_client;
			for (int i = 0; i < clients.size(); i++) {
				if (FD_ISSET((int)clients[i], &readfd)) {   //���ÿһ�������Ƿ����
					char data[1024] = { 0 };
					recv(clients[i], data, 1023, 0);
					string recvdata = data;
					if (recvdata == "quit")
						quit_client.push_back(clients[i]);
					else
						cout << "����" << inet_ntoa(saddr[clients[i]].sin_addr) << ":" << data << endl;
				}
			}

			//�ر��˳����׽���
			if (!quit_client.empty()) {
				for (int i = 0; i < quit_client.size(); i++) {
					cout << "�ͻ���" << inet_ntoa(saddr[quit_client[i]].sin_addr) << "�Ѿ��˳���ʣ����������"
						<< quit_client.size() - 1 << endl;
					closesocket(quit_client[i]);
					vector<SOCKET>::iterator it = find(clients.begin(), clients.end(), quit_client[i]);
					clients.erase(it);
				}
			}
		}


	}
}

//pollfd�ṹ��
//struct pollfd{
//       int fd;//�ļ�������
//       short events;//��������¼�
//      short revents;//ʵ�ʷ��ص��¼�
//   }
/*
typedef std::vector<struct pollfd> PollFdList;
void IOmode::poll_mode()
{
	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(server, (sockaddr*)&serveraddr, sizeof(serveraddr));
	listen(server, 5);

	unsigned long ul = 1;//0:����������:������
	//����socket I/O����ģʽ
	int nret = ioctlsocket(server, FIONBIO, (unsigned long*)&ul);

	//poll
	struct pollfd pfd;
	pfd.fd = server;
	pfd.events = POLLIN;//��עpollin�¼����������¼��ɶ�

	PollFdList pollfds;//����һ����̬���飨������
	pollfds.push_back(pfd);//���ļ���������ӵ���������

	int nready;

	struct sockaddr_in clientaddr;
	socklen_t clientlen;
	int clientfd;

	//�ͻ�����Ϣ
	int num = 0;  //��ǰ������
	map<SOCKET, sockaddr_in> saddr;  //�ͻ�����ַ��Ϣ

	while (true)
	{
		//tiomoutΪ��ֵ�������޵ȴ�
		nready = WSAPoll(&(*pollfds.begin()), pollfds.size() - 1, -1);
		if (WSAGetLastError() == WSAEINVAL) cout << "erro";
		if (nready == 0)
			continue;

		clientfd = accept(server, (sockaddr*)&clientaddr, &clientlen);

		//�Ѽ��������¼�����Ķ�����
		pfd.fd = clientfd;
		pfd.events = POLLIN;
		pfd.revents = 0;         //Ŀǰ��û���κ��¼����أ���Ϊ��
		pollfds.push_back(pfd);
		--nready;

		//����һ�¿ͻ�������Ϣ
		saddr[clientfd] = clientaddr;
		num++;

		cout << "�ͻ���" << inet_ntoa(clientaddr.sin_addr) << "�Ѽ�������..."<< std::endl;

		if (nready == 0)//�¼�����������
			continue;

		for (vector<pollfd>::iterator it = pollfds.begin() + 1; it != pollfds.end() && nready > 0; ++it)
		{
			if (it->revents & POLLIN)//�����pollin�¼�
			{
				--nready;
				clientfd = it->fd;
				char buf[1024] = { 0 };
			
				recv(clientfd, buf, 1024, 0);

				if (buf == "quit") {
					num--;
					cout << "�ͻ���" << inet_ntoa(saddr[clientfd].sin_addr) << "�ѶϿ�.."
						<< "��ǰʣ��������Ϊ��" << num;
					closesocket(clientfd);
				}

				std::cout << buf << endl;  //��ӡ�յ�����Ϣ����
			}
		}
	}
}
*/