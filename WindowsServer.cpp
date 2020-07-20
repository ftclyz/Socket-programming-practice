#include "pch.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <Winerror.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

int main() {
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		return 1;
	}

	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in serveraddr;
	serveraddr.sin_addr.s_addr = inet_addr("192.168.1.9");
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(1300);

	bind(server, (sockaddr *)&serveraddr, sizeof(serveraddr));

	listen(server, 5);
	std::cout << "linsten......" << std::endl;

	//accept处理监听到的端口信息，返回客户机的信息，如果你对此不感兴趣，那就不用把信息保存下来。
	sockaddr_in clientaddr;
	int length = sizeof(clientaddr);
	SOCKET client = accept(server, (sockaddr *)&clientaddr, &length);

	char recvData[1024];
	recv(client, recvData, 1023, 0);
	std::cout << recvData << std::endl;

	WSACleanup();
}