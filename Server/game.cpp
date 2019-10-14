#include "game.h"
#include <vector>

namespace codes {
	char newPlayer = 3;//for others get id newplayer
	char getId = 4;//get new player id;
	char getLobbyInfo = 5;//get lobby nicks itp
	char disconnect = 6;//disconnect player of this id 
}//TODO naprawnie deadlocka broadcast disconnect 
game::game(SOCKET listen, SOCKET leader_, std::u32string nick, std::u32string id)
{
	for (int i = 0; i < 9; i++) {
		free[i] = true;
	}
	lobbyId = id;
	leader = leader_;
	clients[leader].nick = nick;
	clients[leader].id = 1;
	free[1] = false;
	listenSocket = listen;
	FD_SET(listenSocket, &fds);
	FD_SET(leader, &fds);
}
bool game::broadCast(SOCKET socket, char* buff, int len)
{
	std::vector<std::map<SOCKET, slot>::iterator> toDelete;
	for (auto it = clients.begin(); it != clients.end(); it++)
	{
		if (it->first != socket)
		{
			if (!sendLen(it->first, buff, len))
				toDelete.push_back(it);
		}
	}
	for (auto& x : toDelete)
		disconnect(x->first);
	return true;
}
bool game::computeNewClientData(SOCKET socket, std::u32string & nick)
{
	std::u32string code;
	std::u32string newLobby;
	std::u32string lobbyId;
	mbstate_t state;
	printf("all : %s\n", buff);
	code = decode(buff + 4, 20);
	newLobby = decode(buff + 88, 1);
	nick = decode(buff + 96, 0);
	lobbyId = decode(buff + 220, 0);
	if (passCode != code)
	{
		printf("wrong code: \n");
		return false;
	}
	printf("code is ok\n");
	return true;
}
bool game::acceptNewClient() {//TODO dorobienie zwracania kodu b��d�w
	int i = 0;
	for (i = 1; i < 9; i++) {
		if (free[i]) {
			break;
		}
	}
	int ID = i;
	if (!ID)
		return false;
	printf("try to accept chat client\n");
	SOCKET client = accept(listenSocket, nullptr, nullptr);
	if (client == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(client);
		return false;
	}

	char playerId;
	char coding;
	int iResult = receiveLen(client, buff, coding, playerId, 2, 0);//wait 2 sec
	if (iResult <= 0)
	{
		printf("receive failed with error: %d\n", WSAGetLastError());
		closesocket(client);
		return false;
	}
	std::u32string nick;
	if (!computeNewClientData(client, nick)) {
		closesocket(client);
		return false;
	}
	for (auto it = clients.begin(); it != clients.end(); it++) {
		if (it->second.nick == nick) {
			closesocket(client);
			return false;
		}
	}
	free[ID] = false;
	clients[client].nick = nick;
	clients[client].id = ID;
	FD_SET(client, &fds);
	//send new id to player
	addMessagePrefix(buff, 1, codes::getId, ID);
	if (!sendLen(client, buff, 4)) {
		//todo
	}
	//sent all info (about all players in lobby) to new client
	addMessagePrefix(buff, 1,codes::getLobbyInfo, clients.size());
	if (!sendLen(client, buff, 4)) {
		//todo
	}
	for (auto it = clients.begin(); it != clients.end(); it++) {
		if (it->first != client) {
			int size = it->second.nick.size() * 4;
			addMessagePrefix(buff, size, 1, it->second.id);
			code(it->second.nick, buff + 4);
			if (!sendLen(client, buff, size + 4)) {
				//TODO co zrobic ???
			}
		}
	}
	//sent to all players info about new player
	addMessagePrefix(buff, nick.size() * 4, codes::newPlayer, ID);
	code(nick, buff + 4);
	if (broadCast(client, buff, nick.size() * 4 + 4))
	{
		//TODO
	}
}
void game::disconnect(SOCKET socket) {
	if (clients.find(socket) != clients.end()) {
		int id = clients[socket].id;
		free[clients[socket].id] = true;
		clients.erase(socket);
		closesocket(socket);
		FD_CLR(socket, &fds);
		addMessagePrefix(buff, 1, 6, id);
		broadCast(socket, buff, 4);
	}
}
states game::waiting()
{
	while (true) {
		if (!clients.size())
			return states::kill;
		fd_set copy = fds;
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);
		if (socketCount == SOCKET_ERROR) {
			printf("select failed with error: %d\n", WSAGetLastError());
			continue;
		}
		printf("after xd\n");
		for (int i = 0; i < socketCount; i++) {
			printf("in for %d\n", i);
			SOCKET sock = copy.fd_array[i];
			if (sock == listenSocket) {//listen
				if (!acceptNewClient())
					continue;
			}///TODO if leared is disconnected what happens?????
			else {//get message
				int i = recv(sock, buff, 4, 0);
				if (i == 0) //TODO
					disconnect(sock);
			}
		}
	}
	return states();
}

game::~game()
{
	for (int i = 0; i < fds.fd_count; i++)
	{
		SOCKET socket = fds.fd_array[i];
		closesocket(socket);
	}
}
