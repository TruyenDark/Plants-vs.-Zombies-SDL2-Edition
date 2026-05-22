#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib>

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
    int row;
    int col;
};
struct Zombies {
    SDL_Rect rect;
    int id;
    int hp;
    int speed;
    bool eat = false;
    unsigned int moveDelay;
    unsigned int nextMoveTime;
    int row;
    int damage;
    void move() {
        this->rect.x -= this->speed;
    }
    void tryMoveZombie() {
        if (SDL_GetTicks() >= this->nextMoveTime && this->eat == false) {
            this->move();
            this->nextMoveTime = SDL_GetTicks() + moveDelay;
        }
    }
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
struct SeedSlots {
    SDL_Rect rect;
    int plantType;
    int cost;
    bool isReady;
};
struct Yard {
    GridCells plots[5][9];
    std::vector<Plants> plant_list;
    std::vector<Zombies> zombie_list;
    std::vector<Bullets> bullet_list;
    std::vector<Suns> sun_list;
    std::vector<SeedSlots> seed_bar;
    int sunPoints = 50;
    int selectCard = 0;
    int zombieSpawnDelay = 12000;
    int nextZombieSpawnTime = SDL_GetTicks() + zombieSpawnDelay;
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
void initSeedBar(Yard& yard) {
    Vector2 vector2 = {400, 0};
    for (int i = 0; i < 11; i++) {
        SeedSlots card = {{vector2.x, vector2.y, 150, 175}, (i + 1), 100, true};
        yard.seed_bar.push_back(card);
        vector2.x += 150;
    }
}
void renderSeedBar(SDL_Renderer* renderer, Yard& yard) {
    for (int i = 0; i < 11; i++) {
        SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
        SDL_RenderDrawRect(renderer, &yard.seed_bar[i].rect);
    }
}
void spawnZombie(Yard& yard) {
    int random_row = rand() % 5;
    Zombies zombie = {{(yard.plots[random_row][8].rect.x + yard.plots[random_row][8].rect.w), 0, 175/2, 200}, 0, 100, 16, false, 500, (SDL_GetTicks() + 500), random_row, 25};
    zombie.rect.y += (yard.plots[random_row][8].rect.y + yard.plots[random_row][8].rect.h) - (zombie.rect.y + zombie.rect.h);
    yard.zombie_list.push_back(zombie);
}
void deleteObject(Yard& yard, int index, int type) {
    if (type == 1) {
        yard.plant_list.erase(yard.plant_list.begin() + index);
    } else if (type == 2) {
        yard.zombie_list.erase(yard.zombie_list.begin() + index);
    } else if (type == 3) {
        yard.bullet_list.erase(yard.bullet_list.begin() + index);
    } else if (type == 4) {
        yard.sun_list.erase(yard.sun_list.begin() + index);
    } else {
        return;
    }
}
bool deleteZombiesOnEntering(Yard& yard, int index) {
    if ((yard.zombie_list[index].rect.x + yard.zombie_list[index].rect.w) <= 0) {
        yard.zombie_list.erase(yard.zombie_list.begin() + index);
        return true;
    } else {
        return false;
    }
}
void trySpawnZombie(Yard& yard) {
    if (SDL_GetTicks() >= yard.nextZombieSpawnTime) {
        spawnZombie(yard);
        yard.nextZombieSpawnTime = SDL_GetTicks() + yard.zombieSpawnDelay;
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
    initSeedBar(yard);
    
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int xFinger = event.button.x;
                int yFinger = event.button.y;
                for (int i = 0; i < yard.seed_bar.size(); i++) {
                    if (xFinger > yard.seed_bar[i].rect.x && xFinger <= (yard.seed_bar[i].rect.x + yard.seed_bar[i].rect.w) && yFinger >= yard.seed_bar[i].rect.y && yFinger <= (yard.seed_bar[i].rect.y + yard.seed_bar[i].rect.h)) {
                        yard.selectCard = yard.seed_bar[i].plantType;
                        break;
                    }
                }
                if (yard.selectCard > 0) {
                    for (int i = 0; i < 5; i++) {
                        for (int j = 0; j < 9; j++) {
                            if (xFinger >= yard.plots[i][j].rect.x && xFinger <= (yard.plots[i][j].rect.x + yard.plots[i][j].rect.w) && yFinger >= yard.plots[i][j].rect.y && yFinger <= (yard.plots[i][j].rect.y + yard.plots[i][j].rect.h) && yard.plots[i][j].hasPlant == false) {
                                yard.plots[i][j].hasPlant = true;
                                Plants plant = {{yard.plots[i][j].rect.x, yard.plots[i][j].rect.y, yard.plots[i][j].rect.w, yard.plots[i][j].rect.h}, 100, 1000, yard.selectCard, i, j};
                                yard.plant_list.push_back(plant);
                                yard.selectCard = 0;
                                break;
                            }
                        }
                    }
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        trySpawnZombie(yard);
        renderYard(renderer, yard);
        renderSeedBar(renderer, yard);
        for (int i = 0; i < yard.plant_list.size(); i++) {
            if (yard.plant_list[i].id == 1) {
                SDL_SetRenderDrawColor(renderer, 197, 160, 89, 255);
                SDL_RenderFillRect(renderer, &yard.plant_list[i].rect);
                SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
                SDL_RenderDrawRect(renderer, &yard.plant_list[i].rect);
            } else if (yard.plant_list[i].id == 2) {
                SDL_SetRenderDrawColor(renderer, 143, 254, 9, 255);
                SDL_RenderFillRect(renderer, &yard.plant_list[i].rect);
                SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
                SDL_RenderDrawRect(renderer, &yard.plant_list[i].rect);
            } else if (yard.plant_list[i].id == 3) {
                SDL_SetRenderDrawColor(renderer, 153, 0, 0, 255);
                SDL_RenderFillRect(renderer, &yard.plant_list[i].rect);
                SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
                SDL_RenderDrawRect(renderer, &yard.plant_list[i].rect);
            } else if (yard.plant_list[i].id == 4) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
                SDL_RenderFillRect(renderer, &yard.plant_list[i].rect);
                SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
                SDL_RenderDrawRect(renderer, &yard.plant_list[i].rect);
            } else if (yard.plant_list[i].id == 5) {
                SDL_SetRenderDrawColor(renderer, 255, 110, 0, 255);
                SDL_RenderFillRect(renderer, &yard.plant_list[i].rect);
                SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
                SDL_RenderDrawRect(renderer, &yard.plant_list[i].rect);
            } else if (yard.plant_list[i].id == 6) {
                SDL_SetRenderDrawColor(renderer, 138, 43, 226, 255);
                SDL_RenderFillRect(renderer, &yard.plant_list[i].rect);
                SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
                SDL_RenderDrawRect(renderer, &yard.plant_list[i].rect);
            } else if (yard.plant_list[i].id == 7) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 127, 255);
                SDL_RenderFillRect(renderer, &yard.plant_list[i].rect);
                SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
                SDL_RenderDrawRect(renderer, &yard.plant_list[i].rect);
            } else if (yard.plant_list[i].id == 8) {
                SDL_SetRenderDrawColor(renderer, 30, 30, 36, 255);
                SDL_RenderFillRect(renderer, &yard.plant_list[i].rect);
                SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
                SDL_RenderDrawRect(renderer, &yard.plant_list[i].rect);
            } else if (yard.plant_list[i].id == 9) {
                SDL_SetRenderDrawColor(renderer, 242, 175, 41, 255);
                SDL_RenderFillRect(renderer, &yard.plant_list[i].rect);
                SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
                SDL_RenderDrawRect(renderer, &yard.plant_list[i].rect);
            } else if (yard.plant_list[i].id == 10) {
                SDL_SetRenderDrawColor(renderer, 168, 230, 207, 255);
                SDL_RenderFillRect(renderer, &yard.plant_list[i].rect);
                SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
                SDL_RenderDrawRect(renderer, &yard.plant_list[i].rect);
            } else if (yard.plant_list[i].id == 11) {
                SDL_SetRenderDrawColor(renderer, 255, 75, 75, 255);
                SDL_RenderFillRect(renderer, &yard.plant_list[i].rect);
                SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
                SDL_RenderDrawRect(renderer, &yard.plant_list[i].rect);
            }
        }
        for (int i = 0; i < yard.zombie_list.size(); i++) {
            SDL_SetRenderDrawColor(renderer, 255,255,0,255);
            SDL_RenderFillRect(renderer, &yard.zombie_list[i].rect);
            SDL_SetRenderDrawColor(renderer, 0,0,0,255);
            SDL_RenderDrawRect(renderer, &yard.zombie_list[i].rect);
        }
        for (int i = 0; i < yard.zombie_list.size(); i++) {
            yard.zombie_list[i].tryMoveZombie();
            if (deleteZombiesOnEntering(yard, i) == true) {
                i--;
            }
        }
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
