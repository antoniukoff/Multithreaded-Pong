//#include "SDL.h"
#pragma once
#include <SDL.h>

class Player {
private:
    SDL_Point velocity{};
    int isMoving;
    int SCREEN_WIDTH;
    int SCREEN_HEIGHT;
public:
    SDL_Rect paddle{};
    Player() = default;
    Player(int screenWidth, int screenHeight);

    SDL_Rect getPaddle() { return paddle; }

    void innit(bool isPlayer1);

    void move(bool up);
    void stopMoving();

    void render(SDL_Renderer *renderer);

    void update();
    void updatePlayerPaddle(int x, int y, int w, int h);
};
