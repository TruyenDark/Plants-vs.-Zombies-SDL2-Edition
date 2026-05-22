#include <SDL2/SDL.h>
#include <iostream>
#include <vector>

const int SCREEN_WIDTH = 2048;
const int SCREEN_HEIGHT = 1080;

struct Vector2 {
    int x;
    int y;
};
struct GridCells {
    SDL_Rect rect;
    int row;
    int col;
    bool hasPlant = false;
};
struct Plants {
    SDL_Rect rect;
    int hp;
    int cooldown;
    int id;
};
struct Zombies {
    SDL_Rect rect;
    int id;
    int hp;
    int speed;
    bool eat = false;
};
struct Bullets {
    SDL_Rect rect;
    int id;
    int damage;
    int speed;
};
struct Suns {
    SDL_Rect rect;
};
struct Yard {
    GridCells plots[5][9];
    std::vector<Plants> plant_list;
    std::vector<Zombies> zombie_list;
    std::vector<Bullets> bullet_list;
    std::vector<Suns> sun_list;
    int sunPoints = 50;
    int selectCard = 0;
};

void initYard(Yard& yard) {
    Vector2 vector2 = {400, 905};
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 9; j++) {
            yard.plots[i][j] = {{vector2.x, vector2.y, 175, 175}, i, j, false};
            vector2.x += 175;
        }
        vector2.x = 400;
        vector2.y -= 175;
    }
}
void renderYard(SDL_Renderer* renderer, Yard& yard) {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 9; j++) {
            if (((i + j) % 2) == 0) {
                SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
                SDL_RenderFillRect(renderer, &yard.plots[i][j].rect);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 50, 205, 50, 255);
                SDL_RenderFillRect(renderer, &yard.plots[i][j].rect);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "SDL không thể khởi tạo! Lỗi: " << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_SetHint("SDL_HINT_ANDROID_TRAP_BACK_BUTTON", "0"); 
    SDL_SetHint("SDL_ANDROID_BLOCK_ON_PAUSE", "1");
    SDL_SetHint("SDL_HINT_VIDEO_WIN_D3DCOMPILER", "0");
    SDL_Window* window = SDL_CreateWindow(
        "Plants vs. Zombies Classic Simulation",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        (SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP)
    );

    if (window == nullptr) {
        std::cout << "Không thể tạo cửa sổ! Lỗi: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        std::cout << "Không thể tạo Renderer! Lỗi: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    Yard yard;
    initYard(yard);
    
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        renderYard(renderer, yard);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
