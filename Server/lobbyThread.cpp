#pragma once
#include "lobbyThread.h"
#include "MainThreadListener.h"
#include "chat.h"
#include "game.h"
std::mutex mut;

std::map<std::u32string, player> mapaLobby;

#define DEFAULT_BUFLEN 512
void ChatThread(SOCKET socket_, std::u32string lobbyId_)
{
	//main listener tylko init
	MainThreadListener listener(socket_, lobbyId_);
	int port = listener.createListenSocket();
	if (!port) { ; }//todo
	std::string portStr = std::to_string(port);
	mut.lock();
	mapaLobby[lobbyId_].chatPort = port;
	mut.unlock();
	chat Chat(listener.ListenSocket, lobbyId_, portStr);
	Chat.run();
}

void lobbyThread(SOCKET socket_, std::u32string lobbyId, std::u32string leader)//, std::mutex, std::map<std::string, int>)
{

	MainThreadListener listener(socket_, lobbyId);
	//init listener and connect to leader
	int port = listener.createListenSocket();
	if (!port) { ; }//TODO
	mut.lock();
	mapaLobby[lobbyId].lobbyPort = port;
	mut.unlock();
	//przeniesienie kodu z tworzenia listen socketa w chatThread
	if (!listener.acceptLeaderConnection()) { ; }//TODO
	std::thread chat(ChatThread, listener.ClientSocket, lobbyId);//TODO implement games states and lock lobby
	//oczekiwanie na potwierdzenia leadera;
	game Game(listener.ListenSocket, listener.ClientSocket, leader, lobbyId);
	Game.waitForLeaderAccept();
	states state = states::waiting;
	try {
		while (true) {
			switch (state) {
			case states::waiting: //waiting for players, settings and for game start
				state = Game.waitingF();
				break;
			case states::starting: //waiting for players, settings and for game start
				state = Game.startingF();
				break;
			case states::questionInit: //waiting for players, settings and for game start
				state = Game.questionInitF();
				break;
			case states::question: //waiting for players, settings and for game start
				state = Game.questionF();
				break;
			case states::choseInit: //waiting for players, settings and for game start
				state = Game.choseinitF();
				break;
			case states::chose: //waiting for players, settings and for game start
				state = Game.choseF();
				break;
			case states::summing: //waiting for players, settings and for game start
				state = Game.summingF();
				break;
			case states::kill:
				chat.join();
				mut.lock();
				mapaLobby.erase(lobbyId);
				mut.unlock();
				return;
			default:
				break;
			}
		}
	}
	catch (...) {
		char buff[10] = { 30,1,1,0 };//30 lobby hak crushed;
		Game.broadCast(0, buff, 4, true);
		return;
	}
	return;
}


