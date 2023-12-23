#include "Server.h"
#include "Game.h"
#include <string>
#include <thread>

Server::Server(Game* game_): udpSocket(nullptr),
	serverSocket(nullptr),
	clientSocket(nullptr),
	receivePacket(nullptr),
	player1score(0), 
	player2score(0), 
	combinedText(""),
	playerConnected(false),
	INetwork(game_->getRenderer(),
		Player(),
		Player(game_->getScreenWidth(), game_->getScreenHeight()),
		Ball(game_->getScreenWidth(), game_->getScreenHeight()),
		game_->getScreenWidth(),
		game_->getScreenHeight())
{}

void Server::initialize(bool& quit)
{

	udpSocket = SDLNet_UDP_Open(8080);
	if (!udpSocket) {
		throw std::runtime_error("Failed to create UDP socket");
	}

	receivePacket = SDLNet_AllocPacket(1024);
	if (!receivePacket) {
		throw std::runtime_error("Failed to allocate the UDP packet");
	}

	IPaddress ip;
	SDLNet_ResolveHost(&ip, NULL, 12345);
	serverSocket = SDLNet_TCP_Open(&ip);

	printf("ip address: %d\n", ip.host);
	if (!serverSocket) {
		throw std::runtime_error("Failed to open TCP socket");
	}
	
	font = TTF_OpenFont("resources/arial.ttf", 50);
	if (!font) {
		throw std::runtime_error("Failed to load font");
	}

	SDL_Color textColor = { 0, 0, 0, 0xFF }; // Adjust the text color as needed
	textSurface = TTF_RenderText_Solid(font, "PONG IN SDL2!", textColor);
	if (!textSurface) {
		throw std::runtime_error("Failed to create text surface");
	}

	// Create text texture
	textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	if (!textTexture) {
		throw std::runtime_error("Failed to create text texture");
	}

	ball.innit();
	serverPlayer.innit(true);

	receiveClientPositionThread = std::thread(&NetworkingManager::receiveClientPosition, std::ref(quit),
		udpSocket, std::ref(receivePacket), std::ref(clientPlayer.paddle.x), std::ref(clientPlayer.paddle.y), std::ref(clientPlayer.paddle.w), std::ref(clientPlayer.paddle.h));

	sendServerBallPositionThread = std::thread(&NetworkingManager::sendServerBallPosition, std::ref(quit),
		udpSocket, std::ref(receivePacket->address), std::ref(serverPlayer.paddle.x), std::ref(serverPlayer.paddle.y), std::ref(serverPlayer.paddle.w), std::ref(serverPlayer.paddle.h),
		std::ref(ball.ball.x), std::ref(ball.ball.y), std::ref(ball.ball.w), std::ref(ball.ball.h));

	sendServerScoreThread = std::thread(&NetworkingManager::sendScore, std::ref(quit), std::ref(clientSocket), std::ref(combinedText));
}

void Server::cleanup()
{
	receiveClientPositionThread.join();
	sendServerBallPositionThread.join();
	sendServerScoreThread.join();
	SDLNet_FreePacket(receivePacket);
	SDLNet_UDP_Close(udpSocket);
	SDLNet_TCP_Close(serverSocket);
}

void Server::handleEvents(SDL_Event& e)
{
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
		switch (e.key.keysym.sym) {
		case SDLK_w:
			serverPlayer.move(true);
			break;
		case SDLK_s:
			serverPlayer.move(false);
			break;
		case SDLK_SPACE:
			if (clientSocket) {
				playerConnected = true;
				combinedText = "Game started";
			}
			break;
		}
	}
	if (e.type == SDL_KEYUP && e.key.repeat == 0) {
		switch (e.key.keysym.sym) {
		case SDLK_w:
			serverPlayer.stopMoving();
			break;
		case SDLK_s:
			serverPlayer.stopMoving();
			break;
		}
	}
}

void Server::update()
{
	if (!clientSocket) {
		clientSocket = SDLNet_TCP_Accept(serverSocket);
	}

	// Update game objects (e.g., player, ball, background)
	serverPlayer.update();

	if (playerConnected) {
		ball.update();

		if (player1score == MAX_SCORE || player2score == MAX_SCORE) {
			reset();
		}

		if (checkForPaddleCollision(serverPlayer.getPaddle()) ||
			checkForPaddleCollision(clientPlayer.getPaddle())) {
			int curXVelocity = ball.getVelocity().x;
			ball.setVelocity({ -curXVelocity, ball.getVelocity().y });
		}
		else {
			checkForWallCollision();
		}

		if (player1score == MAX_SCORE || player2score == MAX_SCORE) {
			reset();
		}
	}
}

bool Server::checkForPaddleCollision(SDL_Rect paddle)
{
	if (ball.getBall().x + ball.getBall().w >= paddle.x &&
		ball.getBall().x <= paddle.x + paddle.w &&
		ball.getBall().y + ball.getBall().h >= paddle.y &&
		ball.getBall().y <= paddle.y + paddle.h) {
		return true;
	}
	return false;
}

void Server::checkForWallCollision()
{
	// Check for collision with the top and bottom of the screen
	if (ball.getBall().y <= 0 || ball.getBall().y + ball.getBall().h >= SCREEN_HEIGHT) {
		int curYVelocity = ball.getVelocity().y;
		ball.setVelocity({ ball.getVelocity().x, -curYVelocity });
	}

	// Check for collision with the left and right sides of the screen
	if (ball.getBall().x + ball.getBall().w <= 0) {
		// Ball passed the left side of the screen
		// Add points and reset ball position
		int score = getPlayer2Score();
		setPlayer2Score(score + 1); // Add points to player B (or your scoring mechanism)
		ball.innit();   // Reset the ball's position to the center
	}
	else if (ball.getBall().x >= SCREEN_WIDTH) {
		// Ball passed the right side of the screen
		// Add points and reset ball position
		int score = getPlayer1Score();
		setPlayer1Score(score + 1);
		//addPointsToPlayerA();  // Add points to player A (or your scoring mechanism)
		ball.innit();    // Reset the ball's position to the center
	}
	// Convert player scores to strings
	std::string player1ScoreStr = std::to_string(getPlayer1Score());
	std::string player2ScoreStr = std::to_string(getPlayer2Score());
	// Combine the scores with the main text
	combinedText = player1ScoreStr + " : " + player2ScoreStr;
}

void Server::render()
{
	// Initialize renderer color white for the background
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(renderer);

	serverPlayer.render(renderer);

	if (clientSocket) {
		clientPlayer.render(renderer);
	}
	if (playerConnected == true) {
		ball.render(renderer);
		renderText();
	}
	else {
		renderStartText();
	}

	SDL_RenderPresent(renderer);
}


void Server::renderText()
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

	// Create a new text surface with the combined text
	SDL_Surface* newTextSurface = TTF_RenderText_Solid(font, combinedText.c_str(), { 0, 0, 0, 0xFF });
	if (!newTextSurface) {
		std::cout << "TTF_RenderText_Solid() failed: " << TTF_GetError() << std::endl;
		return;
	}

	// Create a new texture from the updated surface
	SDL_Texture* newTextTexture = SDL_CreateTextureFromSurface(renderer, newTextSurface);
	if (!newTextTexture) {
		std::cout << "SDL_CreateTextureFromSurface() failed: " << SDL_GetError() << std::endl;
		SDL_FreeSurface(newTextSurface);
		return;
	}

	// Set the position for the updated text
	int textWidth = newTextSurface->w;
	int centerX = (SCREEN_WIDTH - textWidth) / 2;
	textRect = { centerX, 0, textWidth, newTextSurface->h };

	// Render the updated text
	SDL_RenderCopy(renderer, newTextTexture, NULL, &textRect);

	// Clean up the old text surface and texture
	SDL_FreeSurface(textSurface);
	SDL_DestroyTexture(textTexture);

	// Update the text surface and texture
	textSurface = newTextSurface;
	textTexture = newTextTexture;
}

void Server::renderStartText()
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

	SDL_Surface* startTextSurface = nullptr;

	// Create a new text surface with the desired text
	if (!clientSocket) {
		startTextSurface = TTF_RenderText_Solid(font, "Wait for client to join", { 0, 0, 0, 0xFF });
	} else {
		startTextSurface = TTF_RenderText_Solid(font, "Press any key to start", { 0, 0, 0, 0xFF });
	}

	if (!startTextSurface) {
		std::cout << "TTF_RenderText_Solid() failed: " << TTF_GetError() << std::endl;
		return;
	}

	// Create a new texture from the text surface
	SDL_Texture* startTextTexture = SDL_CreateTextureFromSurface(renderer, startTextSurface);
	if (!startTextTexture) {
		std::cout << "SDL_CreateTextureFromSurface() failed: " << SDL_GetError() << std::endl;
		SDL_FreeSurface(startTextSurface);
		return;
	}

	// Set the position for the start text (centered horizontally and vertically)
	int textWidth = startTextSurface->w;
	int textHeight = startTextSurface->h;
	int centerX = (SCREEN_WIDTH - textWidth) / 2;
	int centerY = (SCREEN_HEIGHT - textHeight) / 2;
	SDL_Rect startTextRect = { centerX, centerY, textWidth, textHeight };

	// Render the start text
	SDL_RenderCopy(renderer, startTextTexture, NULL, &startTextRect);

	// Clean up the text surface and texture
	SDL_FreeSurface(startTextSurface);
	SDL_DestroyTexture(startTextTexture);
}

void Server::reset() {
	combinedText = "Game ended";
	player1score = 0;
	player2score = 0;
	serverPlayer.innit(true);
	ball.innit();
	playerConnected = false;
}