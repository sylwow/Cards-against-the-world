

#pragma once
#include"serverUtils.h"
#include<map>
#define DEFAULT_PORT "3000"
struct client
{
	std::u32string nick;
	char buff[LEN];
	int all;
	int received;
};

class chat: public serverUtils
{
protected:
	char buff[LEN];
	char rcvbuff[LEN];
	std::map<SOCKET, client> clients;
	fd_set fds;
	SOCKET listenSocket;
	std::string port;
	std::u32string lobbyId;
public:
	chat(SOCKET socket_, std::u32string lobbyId_, std::string port_);
	~chat();
	void run();
	bool acceptNewClient();
	bool limitedResponseWait(int time, SOCKET socket);
	bool computeNewClientData(SOCKET client);
	bool broadCast(SOCKET socket, std::u32string msg);
	bool broadCast(SOCKET socket, char* buff, int len);
	bool disconnect(SOCKET socket);
};


