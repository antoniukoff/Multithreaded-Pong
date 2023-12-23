#include "Game.h"
#include "Server.h"
#include "Client.h"

const int TARGET_FPS = 60;
const int DELAY_TIME = 1000 / TARGET_FPS;

Game::Game() : window(nullptr),
    renderer(nullptr), 
    SCREEN_WIDTH(800),
    SCREEN_HEIGHT(600)
{}

int Game::initialize() {

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL_Init() failed");
    }
    if (TTF_Init() == -1) {
        throw std::runtime_error("TTF_Init() failed");
    }
    if (SDLNet_Init() < 0) {
        throw std::runtime_error("SDLNet initialization failed");
    }

    window = SDL_CreateWindow("Pong in SDL2",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH, SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        throw std::runtime_error("Failed to create window");
    } 

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
       throw std::runtime_error("Failed to create renderer");
    }

    return 0;
}

void Game::cleanup() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDLNet_Quit();
    TTF_Quit();
    SDL_Quit();
}

void Game::run() {

    if (chooseMode == NotSelected) {
        printf("Select mode\n");
        int mode = 0;
        std::cin >> mode;
        if (mode == 1) {
            printf("You choose to host the game\n");
            chooseMode = Host;
        }
        else if (mode == 2) {
            printf("You choose to join the game\n");
            chooseMode = Join;
        }
        else {
            std::runtime_error("Wrong mode");
        }
    }
    
    INetwork* networking = (chooseMode == Host) ? dynamic_cast<INetwork*>(new Server(this)) : dynamic_cast<INetwork*>(new Client(this));

    bool quit = false;
    networking->initialize(quit);
    while (!quit) {
        Uint32 frameStart = SDL_GetTicks();

        networking->update();

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true; // Exit the loop if the window is closed
            }
            networking->handleEvents(e); // Handle player input
        }

        networking->render();

        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < DELAY_TIME) {
            SDL_Delay(DELAY_TIME - frameTime);
        }
    }
    networking->cleanup();
    delete networking;
    cleanup();
}