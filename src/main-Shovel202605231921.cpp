#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib>

const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 720;

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
    bool shoot = false;
    unsigned int shootDelay = 1500;
    unsigned int nextShootTime = SDL_GetTicks() + shootDelay;
    unsigned int sunProductionTimer = 45000;
    unsigned int sunSpawnTimer = SDL_GetTicks() + sunProductionTimer;
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
    float speed;
    int row;
    int col;
};
struct Suns {
    SDL_Rect rect;
    int sunValue;
    int isFromSky;
    int targetY;
    unsigned int expiryTimer;
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
    unsigned int zombieSpawnDelay = 12000;
    unsigned int nextZombieSpawnTime = SDL_GetTicks() + zombieSpawnDelay;
    unsigned int sunSpawnInterval = 45000;
    unsigned int sunSpawnCooldown = SDL_GetTicks() + sunSpawnInterval;
};

void initYard(Yard& yard) {
    Vector2 vector2 = {400, 600};
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 9; j++) {
            yard.plots[i][j] = {{vector2.x, vector2.y, 120, 120}, i, j, false};
            vector2.x += 120;
        }
        vector2.x = 400;
        vector2.y -= 120;
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
        SeedSlots card = {{vector2.x, vector2.y, 100, 120}, (i + 1), 50, true};
        yard.seed_bar.push_back(card);
        vector2.x += 100;
    }
    SeedSlots shovel = {{vector2.x, vector2.y, 100, 100}, 12, 0, true};
    yard.seed_bar.push_back(shovel);
}
void renderSeedBar(SDL_Renderer* renderer, Yard& yard) {
    for (int i = 0; i < yard.seed_bar.size(); i++) {
        SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
        SDL_RenderDrawRect(renderer, &yard.seed_bar[i].rect);
    }
}
void spawnZombie(Yard& yard) {
    int random_row = rand() % 5;
    Zombies zombie = {{(yard.plots[random_row][8].rect.x + yard.plots[random_row][8].rect.w), 0, 120/2, 140}, 0, 300, 12, false, 500, (SDL_GetTicks() + 500), random_row, 1};
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
void checkAndReloadAmmo(Yard& yard) {
    for (int i = 0; i < yard.plant_list.size(); i++) {
        yard.plant_list[i].shoot = false;
        for (int j = 0; j < yard.zombie_list.size(); j++) {
            if (yard.plant_list[i].row == yard.zombie_list[j].row && (yard.plant_list[i].rect.x + yard.plant_list[i].rect.w) <= yard.zombie_list[j].rect.x) {
                yard.plant_list[i].shoot = true;
                break;
            } else {
                yard.plant_list[i].shoot = false;
            }
        }
        if (yard.plant_list[i].shoot == true && SDL_GetTicks() >= yard.plant_list[i].nextShootTime) {
            Bullets bullet = {{yard.plant_list[i].rect.x + yard.plant_list[i].rect.w, yard.plant_list[i].rect.y, 25, 25}, 0, 25, 1, yard.plant_list[i].row};
            yard.bullet_list.push_back(bullet);
            yard.plant_list[i].nextShootTime = SDL_GetTicks() + yard.plant_list[i].shootDelay;
        }
    }
}
void drawBullet(Yard& yard, SDL_Renderer* renderer) {
    for (int i = 0; i < yard.bullet_list.size(); i++) {
        SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
        SDL_RenderFillRect(renderer, &yard.bullet_list[i].rect);
        SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
        SDL_RenderDrawRect(renderer, &yard.bullet_list[i].rect);
    }
}
void updateBullets(Yard& yard) {
    for (int i = 0; i < yard.bullet_list.size(); i++) {
        yard.bullet_list[i].rect.x += yard.bullet_list[i].speed;
        for (int j = 0; j < yard.zombie_list.size(); j++) {
            if (yard.bullet_list[i].rect.x >= 2048) {
                yard.bullet_list.erase(yard.bullet_list.begin() + i);
                i--;
                break;
            } else if (yard.bullet_list[i].row == yard.zombie_list[j].row && (yard.bullet_list[i].rect.x + yard.bullet_list[i].rect.w) >= yard.zombie_list[j].rect.x) {
                yard.zombie_list[j].hp -= yard.bullet_list[i].damage;
                yard.bullet_list.erase(yard.bullet_list.begin() + i);
                i--;
                if (yard.zombie_list[j].hp <= 0) {
                    yard.zombie_list.erase(yard.zombie_list.begin() + j);
                }
                break;
            }
        }
    }
}
void zombieEat(Yard& yard) {
    for (int i = 0; i < yard.zombie_list.size(); i++) {
        yard.zombie_list[i].eat = false;
        for (int j = 0; j < yard.plant_list.size(); j++) {
            if (yard.zombie_list[i].row == yard.plant_list[j].row && yard.zombie_list[i].rect.x <= (yard.plant_list[j].rect.x + yard.plant_list[j].rect.w) && (yard.zombie_list[i].rect.x + yard.zombie_list[i].rect.w) >= yard.plant_list[j].rect.x) {
                yard.zombie_list[i].eat = true;
                yard.plant_list[j].hp -= yard.zombie_list[i].damage;
                if (yard.plant_list[j].hp <= 0) {
                    yard.plots[yard.plant_list[j].row][yard.plant_list[j].col].hasPlant = false;
                    deleteObject(yard, j, 1);
                    yard.zombie_list[i].eat = false;
                }
                break;
            }
        }
    }
}
void drawPlant(Yard& yard, SDL_Renderer* renderer) {
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
}
void drawZombie(Yard& yard, SDL_Renderer* renderer) {
    for (int i = 0; i < yard.zombie_list.size(); i++) {
        SDL_SetRenderDrawColor(renderer, 255,255,0,255);
        SDL_RenderFillRect(renderer, &yard.zombie_list[i].rect);
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderDrawRect(renderer, &yard.zombie_list[i].rect);
    }
}
void moveZombiesSynchronously(Yard& yard) {
    for (int i = 0; i < yard.zombie_list.size(); i++) {
        yard.zombie_list[i].tryMoveZombie();
        if (deleteZombiesOnEntering(yard, i) == true) {
            i--;
        }
    }
}
void spawnSunflowerSun(Yard& yard) {
    for (int i = 0; i < yard.plant_list.size(); i++) {
        if (yard.plant_list[i].id == 2 && SDL_GetTicks() >= yard.plant_list[i].sunSpawnTimer) {
            Suns sun = {{yard.plant_list[i].rect.x, yard.plant_list[i].rect.y, 75, 75}, 25, false, 0, SDL_GetTicks() + 25000};
            yard.sun_list.push_back(sun);
            yard.plant_list[i].sunSpawnTimer = SDL_GetTicks() + yard.plant_list[i].sunProductionTimer;
        }
    }
}

void spawnSkySun(Yard& yard) {
    if (SDL_GetTicks() >= yard.sunSpawnCooldown) {
        int random_x = rand() % 1480;
        int random_targetY = rand() % 600;
        Suns sun = {{random_x, -120, 75, 75}, 25, true, random_targetY, SDL_GetTicks() + 25000};
        yard.sun_list.push_back(sun);
        yard.sunSpawnCooldown = SDL_GetTicks() + yard.sunSpawnInterval;
    }
}
void updateSkySun(Yard& yard) {
    for (int i = 0; i < yard.sun_list.size(); i++) {
        if (yard.sun_list[i].isFromSky == true) {
            yard.sun_list[i].rect.y += 1;
        }
        if (yard.sun_list[i].rect.y >= yard.sun_list[i].targetY) {
            yard.sun_list[i].isFromSky = false;
        }
        if (SDL_GetTicks() >= yard.sun_list[i].expiryTimer) {
            yard.sun_list.erase(yard.sun_list.begin() + i);
            i--;
        }
    }
}
void drawSun(Yard& yard, SDL_Renderer* renderer) {
    for (int i = 0; i < yard.sun_list.size(); i++) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(renderer, &yard.sun_list[i].rect);
        SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
        SDL_RenderDrawRect(renderer, &yard.sun_list[i].rect);
    }
}
void onCollectSunClick(Yard& yard, int xFinger, int yFinger) {
    for (int i = 0; i < yard.sun_list.size(); i++) {
        if (xFinger >= yard.sun_list[i].rect.x && xFinger <= (yard.sun_list[i].rect.x + yard.sun_list[i].rect.w) && yFinger >= yard.sun_list[i].rect.y && yFinger <= (yard.sun_list[i].rect.y + yard.sun_list[i].rect.h)) {
            yard.sunPoints += yard.sun_list[i].sunValue;
            yard.sun_list.erase(yard.sun_list.begin() + i);
            break;
        }
    }
}
void checkSunOnSeedClick(Yard& yard, int xFinger, int yFinger) {
    for (int i = 0; i < yard.seed_bar.size(); i++) {
        if (xFinger > yard.seed_bar[i].rect.x && xFinger <= (yard.seed_bar[i].rect.x + yard.seed_bar[i].rect.w) && yFinger >= yard.seed_bar[i].rect.y && yFinger <= (yard.seed_bar[i].rect.y + yard.seed_bar[i].rect.h)) {
            if (yard.selectCard == yard.seed_bar[i].plantType) {
                yard.selectCard = 0;
            } else if (yard.sunPoints >= yard.seed_bar[i].cost) {
                yard.selectCard = yard.seed_bar[i].plantType;
            }
            break;
        }
    }
}
void placeOrRemovePlant(Yard& yard, int xFinger, int yFinger) {
    if (yard.selectCard > 0 && yard.selectCard < 12) {
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 9; j++) {
                if (xFinger >= yard.plots[i][j].rect.x && xFinger <= (yard.plots[i][j].rect.x + yard.plots[i][j].rect.w) && yFinger >= yard.plots[i][j].rect.y && yFinger <= (yard.plots[i][j].rect.y + yard.plots[i][j].rect.h) && yard.plots[i][j].hasPlant == false) {
                    yard.plots[i][j].hasPlant = true;
                    Plants plant = {{yard.plots[i][j].rect.x, yard.plots[i][j].rect.y, yard.plots[i][j].rect.w, yard.plots[i][j].rect.h}, 100, 1000, yard.selectCard, i, j, false, 3000, (SDL_GetTicks() + 3000), 45000, (SDL_GetTicks() + 45000)};
                    yard.plant_list.push_back(plant);
                    yard.sunPoints -= yard.seed_bar[(yard.selectCard - 1)].cost;
                    yard.selectCard = 0;
                    return;
                }
            }
        }
    } else if (yard.selectCard == 12) {
        for (int i = 0; i < yard.plant_list.size(); i++) {
            if (xFinger >= yard.plant_list[i].rect.x && xFinger <= (yard.plant_list[i].rect.x + yard.plant_list[i].rect.w) && yFinger >= yard.plant_list[i].rect.y && yFinger <= (yard.plant_list[i].rect.y + yard.plant_list[i].rect.h)) {
                yard.selectCard = 0;
                yard.plots[yard.plant_list[i].row][yard.plant_list[i].col].hasPlant = false;
                yard.plant_list.erase(yard.plant_list.begin() + i);
                return;
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
    initSeedBar(yard);
    
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int xFinger = event.button.x;
                int yFinger = event.button.y;
                checkSunOnSeedClick(yard, xFinger, yFinger);
                placeOrRemovePlant(yard, xFinger, yFinger);
                onCollectSunClick(yard, xFinger, yFinger);
            }
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        trySpawnZombie(yard);
        zombieEat(yard);
        checkAndReloadAmmo(yard);
        moveZombiesSynchronously(yard);
        updateBullets(yard);
        spawnSunflowerSun(yard);
        spawnSkySun(yard);
        updateSkySun(yard);
        renderYard(renderer, yard);
        renderSeedBar(renderer, yard);
        drawZombie(yard, renderer);
        drawPlant(yard, renderer);
        drawBullet(yard, renderer);
        drawSun(yard, renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
