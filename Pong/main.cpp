#include <iostream>
#include <SDL.h>
#include "Game.h"
#include <SDL_net.h>

const int TARGET_FPS = 60;
const int DELAY_TIME = 1000 / TARGET_FPS;


int main(int argc, char* argv[]) {

    Game game;
    game.initialize();
    game.run();
    return 0;
}



