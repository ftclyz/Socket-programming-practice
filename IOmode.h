#pragma once
#include "pch.h"
#include <winsock2.h>
#include <Winerror.h>
#include <string>
#include<iostream>
using namespace std;

class IOmode {
private:
	sockaddr_in serveraddr;
public:
	IOmode(int port, string ipaddr);

	~IOmode();

	void block_mode();

	void select_mode();

	void poll_mode();
};