#include "pch.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <Winerror.h>
#include <iostream>

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

	int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in serveraddr;
	serveraddr.sin_addr.s_addr = inet_addr("192.168.1.9");
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(1300);
	
	connect(client, (sockaddr*)&serveraddr, sizeof(serveraddr));
	
	send(client, "nihao", sizeof("nihao"), 0);

	WSACleanup();
}