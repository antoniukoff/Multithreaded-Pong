#include <SDL_ttf.h>
#include "Player.h"
#include "Ball.h"

enum ChooseMode {
    NotSelected = 0,
    Host = 1,
    Join = 2
};

class Game {
public:
    Game();
    ~Game() = default;
    int initialize();
    void cleanup();
    void run();

    SDL_Renderer *getRenderer() const { return renderer; }
    int getScreenWidth() const { return SCREEN_WIDTH; }
    int getScreenHeight() const { return SCREEN_HEIGHT; }

private:
    int SCREEN_WIDTH;
    int SCREEN_HEIGHT;

    SDL_Window *window;
    SDL_Renderer *renderer;
    ChooseMode chooseMode = NotSelected;
};
