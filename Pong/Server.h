#include <SDL_ttf.h>
#include <iostream>
#include "INetwork.h"
#include "NetworkingManager.h"

class Game;

class Server : public INetwork {
public:
	Server(Game* game_);
	~Server() = default;
	void initialize(bool& quit) override;
	void cleanup() override;
	void handleEvents(SDL_Event &e) override;
	void update() override;
	void render() override;
	void renderText() override;
	void renderStartText() override;
	bool checkForPaddleCollision(SDL_Rect paddle);
	void checkForWallCollision();
	void reset();

	int getPlayer1Score() const { return player1score; }
	int getPlayer2Score() const { return player2score; }

	void setPlayer1Score(int score) {
		player1score = score;
		std::cout << "Player 1 score: " << player1score << std::endl;
	}
	void setPlayer2Score(int score) {
		player2score = score;
		std::cout << "Player 2 score: " << player2score << std::endl;
	}

private:
	UDPsocket udpSocket;
	UDPpacket* receivePacket;
	TCPsocket serverSocket;
	TCPsocket clientSocket;
	NetworkingManager networkingManager;

	int player1score;
	int player2score;
	int MAX_SCORE = 10;
	bool playerConnected;
	std::string combinedText;

	std::thread sendServerBallPositionThread;
	std::thread receiveClientPositionThread;
	std::thread sendServerScoreThread;
};
