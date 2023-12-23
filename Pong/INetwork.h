#pragma once
#include "Player.h"
#include <iostream>
#include <SDL_ttf.h>
#include <SDL_net.h>
#include "Ball.h"

class INetwork
{
public:
    INetwork(SDL_Renderer* renderer_, Player clientPlayer_, Player serverPlayer_, Ball ball_, int screenWidth, int screenHeight) :
        renderer(renderer_),
        clientPlayer(clientPlayer_),
        serverPlayer(serverPlayer_),
        ball(ball_),
        textTexture(nullptr), 
        font(nullptr),
        textSurface(nullptr),
        textRect{},
        SCREEN_WIDTH(screenWidth),
        SCREEN_HEIGHT(screenHeight)
    {}  
     virtual ~INetwork() {
         SDL_FreeSurface(textSurface);
         SDL_DestroyTexture(textTexture);
         TTF_CloseFont(font);
     }
    virtual void initialize(bool &quit) = 0;
    virtual void cleanup() = 0;
    virtual void handleEvents(SDL_Event& e) = 0;
    virtual void update() = 0;
    virtual void render() = 0;
    virtual void renderText() = 0;
    virtual void renderStartText() = 0;

protected:
    int SCREEN_WIDTH;
    int SCREEN_HEIGHT;
    Player clientPlayer; 
    Player serverPlayer;
    Ball ball;

    SDL_Renderer* renderer;
    SDL_Texture* textTexture;
    TTF_Font* font;
    SDL_Surface* textSurface;
    SDL_Rect textRect;
};

