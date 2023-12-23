#pragma once
#include "INetwork.h"
#include "NetworkingManager.h"

class Game;

class Client : public INetwork
{
public:
	Client(Game* game_);
	~Client() = default;
	void initialize(bool& quit) override;
	void cleanup() override;
	void handleEvents(SDL_Event& e) override;
	void update() override;
	void render() override;
	void renderText() override;
	void renderStartText() override;
	NetworkingManager networkingManager;
private:
	
	UDPsocket udpSocket;
	UDPpacket* receivePacket;
	TCPsocket serverSocket;
	IPaddress serverIP;

	bool playerConnected;
	std::string score;

	std::thread sendClientPositionThread;
	std::thread receiveServerBallPositionThread;
	std::thread receiveScoreThread;

};

