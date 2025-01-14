#include "MainThreadListener.h"


extern std::mutex mut;
extern std::map<std::u32string, player> mapaLobby;
MainThreadListener::MainThreadListener(SOCKET socket_, std::u32string lobbyId_)
{
	oldSocket = socket_;
	lobbyId = lobbyId_;
}

int MainThreadListener::createListenSocket()
{
	printf("creating listen socket\n");
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		return 0;
	}
	sockaddr_in addr;
	int size = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(0);
	addr.sin_addr.s_addr = INADDR_ANY;
	printf("binding listen socket with %d port thread\n", 0);
	iResult = bind(ListenSocket, (struct sockaddr*) & addr, size);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		return 0;
	}
	getsockname(ListenSocket, (struct sockaddr*) & addr, &size);
	listenPort = ntohs(addr.sin_port);
	std::string msg = std::to_string(listenPort);
	addMessagePrefix(recvbuf, msg.size(), 0, 0);
	strcpy_s(recvbuf + 4, LEN, msg.c_str());
	if (!sendLen(oldSocket, recvbuf, msg.size() + 4)) {
		printf("send failed with error:\n");
		return 0;
	}
	printf("after sending listen port %d\n", listenPort);
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		return 0;
	}
	return listenPort;
}

bool MainThreadListener::acceptLeaderConnection()
{
	closesocket(oldSocket);
	// Accept a client socket
	printf("accepting with %d port thread\n", listenPort);
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		return false;
	}
	// Wait until timeout or data received.
	printf("accepted with %d port thread\n", listenPort);
	return true;
}


MainThreadListener::~MainThreadListener()
{
}
