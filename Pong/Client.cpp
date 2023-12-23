#include "Client.h"
#include "Game.h"
#include <string>


Client::Client(Game* game_) : udpSocket(nullptr),
	receivePacket(nullptr),
	serverSocket(nullptr),
	serverIP{},
	playerConnected(false),
	INetwork(game_->getRenderer(), 
		Player(game_->getScreenWidth(), game_->getScreenHeight()),
		Player(),
		Ball(),
		game_->getScreenWidth(),
		game_->getScreenHeight())
{}


void Client::initialize(bool& quit)
{
	if (SDLNet_ResolveHost(&serverIP, "127.0.0.1", 8080) < 0) {
		throw std::runtime_error("Failed to resolve the server hostname");
	}

	udpSocket = SDLNet_UDP_Open(0);
	if (!udpSocket) {
		throw std::runtime_error("Failed to create UDP socket");
	}

	receivePacket = SDLNet_AllocPacket(1024);
	if (!receivePacket) {
		throw std::runtime_error("Failed to allocate the UDP packet");
	}

	IPaddress ip;
	SDLNet_ResolveHost(&ip, "127.0.0.1", 12345);
	serverSocket = SDLNet_TCP_Open(&ip);
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

	clientPlayer.innit(false);

	sendClientPositionThread = std::thread(&NetworkingManager::sendClientPosition, std::ref(quit), udpSocket, serverIP,
		std::ref(clientPlayer.paddle.x), std::ref(clientPlayer.paddle.y), std::ref(clientPlayer.paddle.w), std::ref(clientPlayer.paddle.h));

	receiveServerBallPositionThread = std::thread(&NetworkingManager::receiveServerBallPosition, std::ref(quit), udpSocket, receivePacket,
		std::ref(serverPlayer.paddle.x), std::ref(serverPlayer.paddle.y), std::ref(serverPlayer.paddle.w), std::ref(serverPlayer.paddle.h),
		std::ref(ball.ball.x), std::ref(ball.ball.y), std::ref(ball.ball.w), std::ref(ball.ball.h));

	receiveScoreThread = std::thread(&NetworkingManager::receiveScore, std::ref(quit), std::ref(serverSocket), std::ref(playerConnected), std::ref(score));
}

void Client::cleanup()
{
	sendClientPositionThread.join();
	receiveServerBallPositionThread.join();
	receiveScoreThread.join();
	SDLNet_FreePacket(receivePacket);
	SDLNet_UDP_Close(udpSocket);
	SDLNet_TCP_Close(serverSocket);
}

void Client::handleEvents(SDL_Event& e)
{
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
		switch (e.key.keysym.sym) {
		case SDLK_UP:
			clientPlayer.move(true);
			break;
		case SDLK_DOWN:
			clientPlayer.move(false);
			break;
		}
	}
	if (e.type == SDL_KEYUP && e.key.repeat == 0) {
		switch (e.key.keysym.sym) {
		case SDLK_UP:
			clientPlayer.stopMoving();
			break;
		case SDLK_DOWN:
			clientPlayer.stopMoving();
			break;
		}
	}
}

void Client::update()
{
	clientPlayer.update();
}

void Client::render()
{
	// Initialize renderer color white for the background
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(renderer);

	serverPlayer.render(renderer);
	clientPlayer.render(renderer);
	if (playerConnected == false) {
		renderStartText();
	}
	else {
		ball.render(renderer);
		renderText();
	}
	
	SDL_RenderPresent(renderer);
}

void Client::renderText()
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

	// Create a new text surface with the combined text
	SDL_Surface* newTextSurface = TTF_RenderText_Solid(font, score.c_str(), { 0, 0, 0, 0xFF });
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

void Client::renderStartText()
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

	SDL_Surface* startTextSurface = startTextSurface = TTF_RenderText_Solid(font, "Wait for server to start the game", { 0, 0, 0, 0xFF });
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
