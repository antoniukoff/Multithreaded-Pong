#pragma once
#include <SDL.h>

class Ball {

public:
    Ball() = default;
    Ball(int screenWidth, int screenHeight);
    void innit();
    void render(SDL_Renderer *renderer);
    void update();

    SDL_Rect getBall() const { return ball; }
    SDL_Point getVelocity() const { return velocity; }

    void setVelocity(SDL_Point vel) { velocity = vel; }
    void setRect(int x, int y, int w, int h) { ball = { x, y, w, h }; }
    SDL_Rect ball;

private:
    SDL_Point velocity;
    int SCREEN_WIDTH;
    int SCREEN_HEIGHT;

};


