#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include<iostream>
#include <stdbool.h>
#include <stdlib.h> // For random module

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700
#define GROUND_HEIGHT 120
#define GRAVITY 1
#define JUMP_STRENGTH 25
#define MOVE_SPEED 12
#define MAX_JUMPS 4
#define NUM_STONES 20

typedef struct {
    int x, y, size;
} Stone;

typedef struct {
    SDL_Texture* texture;
    SDL_Rect rect;
    int velocity_y;
    int velocity_x;
    int jumpCount;
} Dinosaur;

typedef struct {
    SDL_Texture* texture;
    SDL_Rect rect;
    double velocity_x;
    bool active;
} Ghost;

bool init(SDL_Window** window, SDL_Renderer** renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout<<"SDL Init Error: "<<TTF_GetError()<<std::endl;
        return false;
    }

    *window = SDL_CreateWindow("Jumping Dino", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (*window == NULL) {
        std::cout<<"Window Error: "<<TTF_GetError()<<std::endl;
        SDL_Quit();
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (*renderer == NULL) {
        SDL_DestroyWindow(*window);
        std::cout<<"Renderer Error: "<<TTF_GetError()<<std::endl;
        SDL_Quit();
        return false;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cout<<"Image Init Error: "<<TTF_GetError()<<std::endl;
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return false;
    }

    if (TTF_Init() == -1) {
        std::cout << "TTF_Init: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    return true;
}

SDL_Texture* loadTexture(const char* file, SDL_Renderer* renderer) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, file);
    if (texture == NULL) {
        std::cout<<"Image Load Texture Error: "<<TTF_GetError()<<std::endl;
    }
    return texture;
}

void handleEvents(bool* running, Dinosaur* dino) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            *running = false;
        }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    if (dino->jumpCount < MAX_JUMPS) {
                        dino->velocity_y = -JUMP_STRENGTH;
                        dino->jumpCount++;
                    }
                    break;
                case SDLK_LEFT:
                    dino->velocity_x = -MOVE_SPEED;
                    break;
                case SDLK_RIGHT:
                    dino->velocity_x = MOVE_SPEED;
                    break;
            }
        }
        if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                case SDLK_RIGHT:
                    dino->velocity_x = 0;
                    break;
            }
        }
    }
}

void updateDino(Dinosaur* dino) {
    dino->rect.y += dino->velocity_y;
    dino->rect.x += dino->velocity_x;

    dino->velocity_y += GRAVITY;

    int groundLevel = WINDOW_HEIGHT - GROUND_HEIGHT - dino->rect.h;

    // Prevent the dino from going below the ground
    if (dino->rect.y >= groundLevel) {
        dino->rect.y = groundLevel;
        dino->velocity_y = 0;
        dino->jumpCount = 0;
    }

    // Prevent the dino from jumping above the window
    if (dino->rect.y < 0) {
        dino->rect.y = 0;
        dino->velocity_y = 0; // stop vertical movement
    }

    // Ensure dino does not move out of the window horizontally
    if (dino->rect.x < 0) {
        dino->rect.x = 0;
    } else if (dino->rect.x > WINDOW_WIDTH - dino->rect.w) {
        dino->rect.x = WINDOW_WIDTH - dino->rect.w;
    }
}

void updateGhost(Ghost* ghost) {
    ghost->rect.x += ghost->velocity_x;
    if (ghost->rect.x > WINDOW_WIDTH) {
        ghost->active = false;
    }
}

bool checkCollision(SDL_Rect* a, SDL_Rect* b) {
    return (a->x + a->w > b->x &&
            a->x < b->x + b->w &&
            a->y + a->h > b->y &&
            a->y < b->y + b->h);
}

void renderGrassAndSoil(SDL_Renderer* renderer, SDL_Rect* groundRect, Stone* stones, int numStones) {
    // Draw the soil
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown color for soil
    SDL_Rect soilRect = {groundRect->x, groundRect->y + (groundRect->h / 2), groundRect->w, groundRect->h / 2};
    SDL_RenderFillRect(renderer, &soilRect);

    // Draw the grass
    SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255); // Green color for grass
    SDL_Rect grassRect = {groundRect->x, groundRect->y, groundRect->w, groundRect->h / 2};
    SDL_RenderFillRect(renderer, &grassRect);

    // Add grass blades
    SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255); // Darker green for grass blades
    for (int i = groundRect->x; i < groundRect->w; i += 10) {
        int bladeHeight = rand() % 10 + 5; // Random height for grass blades
        SDL_RenderDrawLine(renderer, i, groundRect->y + (groundRect->h / 2) - bladeHeight, i, groundRect->y + (groundRect->h / 2));
    }

    // Add stones/rocks
    SDL_SetRenderDrawColor(renderer, 105, 105, 105, 255); // Dark gray color for stones
    for (int i = 0; i < numStones; ++i) {
        SDL_Rect stoneRect = {stones[i].x, stones[i].y, stones[i].size, stones[i].size};
        SDL_RenderFillRect(renderer, &stoneRect);
    }
}

void renderBackground(SDL_Renderer* renderer, SDL_Texture* treeTexture, SDL_Texture* cloudTexture, Stone* stones, int numStones) {
    // Sky
    SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255); // Sky blue
    SDL_Rect skyRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT - GROUND_HEIGHT};
    SDL_RenderFillRect(renderer, &skyRect);

    // Ground
    SDL_Rect groundRect = {0, WINDOW_HEIGHT - GROUND_HEIGHT, WINDOW_WIDTH, GROUND_HEIGHT};
    renderGrassAndSoil(renderer, &groundRect, stones, numStones);

    // Render trees
    SDL_Rect treeRect1 = {90, WINDOW_HEIGHT - GROUND_HEIGHT - 160, 180, 180};
    SDL_RenderCopy(renderer, treeTexture, NULL, &treeRect1);

    SDL_Rect treeRect2 = {750, WINDOW_HEIGHT - GROUND_HEIGHT - 190, 250, 250};
    SDL_RenderCopy(renderer, treeTexture, NULL, &treeRect2);

    SDL_Rect treeRect3 = {250, WINDOW_HEIGHT - GROUND_HEIGHT - 130, 180, 180};
    SDL_RenderCopy(renderer, treeTexture, NULL, &treeRect3);

    // Render clouds
    SDL_Rect cloudRect1 = {200, 50, 150, 100};
    SDL_RenderCopy(renderer, cloudTexture, NULL, &cloudRect1);

    SDL_Rect cloudRect2 = {400, 100, 150, 100};
    SDL_RenderCopy(renderer, cloudTexture, NULL, &cloudRect2);

    SDL_Rect cloudRect3 = {700, 50, 130, 100};
    SDL_RenderCopy(renderer, cloudTexture, NULL, &cloudRect3);
}

void render(SDL_Renderer* renderer, Dinosaur* dino, Ghost* ghost, SDL_Texture* treeTexture, SDL_Texture* cloudTexture, Stone* stones, int numStones) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    renderBackground(renderer, treeTexture, cloudTexture, stones, numStones);

    SDL_RenderCopy(renderer, dino->texture, NULL, &dino->rect);
    SDL_RenderCopy(renderer, ghost->texture, NULL, &ghost->rect);

    SDL_RenderPresent(renderer);
}

void cleanUp(SDL_Window* window, SDL_Renderer* renderer, Dinosaur* dino, Ghost* ghost, SDL_Texture* treeTexture, SDL_Texture* cloudTexture, SDL_Texture* menuTexture) {
    SDL_DestroyTexture(dino->texture);
    SDL_DestroyTexture(ghost->texture);
    SDL_DestroyTexture(treeTexture);
    SDL_DestroyTexture(cloudTexture);
    SDL_DestroyTexture(menuTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

bool mainMenu(SDL_Renderer* renderer, SDL_Texture* menuTexture) {
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
        return false;
    }

    SDL_Color textColor = {0, 0, 0, 255};
    SDL_Surface* startSurface = TTF_RenderText_Solid(font, "Start", textColor);
    SDL_Surface* exitSurface = TTF_RenderText_Solid(font, "Exit", textColor);
    if (!startSurface || !exitSurface) {
        std::cout << "Failed to create text surface: " << TTF_GetError() << std::endl;
        TTF_CloseFont(font);
        return false;
    }

    SDL_Texture* startTexture = SDL_CreateTextureFromSurface(renderer, startSurface);
    SDL_Texture* exitTexture = SDL_CreateTextureFromSurface(renderer, exitSurface);
    if (!startTexture || !exitTexture) {
        std::cout << "Failed to create text texture: " << TTF_GetError() << std::endl;
        SDL_FreeSurface(startSurface);
        SDL_FreeSurface(exitSurface);
        TTF_CloseFont(font);
        return false;
    }

    SDL_FreeSurface(startSurface);
    SDL_FreeSurface(exitSurface);

    SDL_Rect startButton = {WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 50, 100, 50};
    SDL_Rect exitButton = {WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 + 50, 100, 50};

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                return false;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (x >= startButton.x && x <= startButton.x + startButton.w &&
                    y >= startButton.y && y <= startButton.y + startButton.h) {
                    running = false;
                    return true;
                }
                if (x >= exitButton.x && x <= exitButton.x + exitButton.w &&
                    y >= exitButton.y && y <= exitButton.y + exitButton.h) {
                    running = false;
                    return false;
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, menuTexture, NULL, NULL);

        SDL_RenderCopy(renderer, startTexture, NULL, &startButton);
        SDL_RenderCopy(renderer, exitTexture, NULL, &exitButton);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(startTexture);
    SDL_DestroyTexture(exitTexture);
    TTF_CloseFont(font);

    return false;
}

bool gameOverMenu(SDL_Renderer* renderer, SDL_Texture* menuTexture) {
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
        return false;
    }

    SDL_Color textColor = {0, 0, 0, 255};
    SDL_Surface* restartSurface = TTF_RenderText_Solid(font, "Restart", textColor);
    SDL_Surface* exitSurface = TTF_RenderText_Solid(font, "Exit", textColor);
    if (!restartSurface || !exitSurface) {
        std::cout << "Failed to create text surface: " << TTF_GetError() << std::endl;
        TTF_CloseFont(font);
        return false;
    }

    SDL_Texture* restartTexture = SDL_CreateTextureFromSurface(renderer, restartSurface);
    SDL_Texture* exitTexture = SDL_CreateTextureFromSurface(renderer, exitSurface);
    if (!restartTexture || !exitTexture) {
        std::cout << "Failed to create text texture: " << TTF_GetError() << std::endl;
        SDL_FreeSurface(restartSurface);
        SDL_FreeSurface(exitSurface);
        TTF_CloseFont(font);
        return false;
    }

    SDL_FreeSurface(restartSurface);
    SDL_FreeSurface(exitSurface);

    SDL_Rect restartButton = {WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 50, 100, 50};
    SDL_Rect exitButton = {WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 + 50, 100, 50};

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                return false;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (x >= restartButton.x && x <= restartButton.x + restartButton.w &&
                    y >= restartButton.y && y <= restartButton.y + restartButton.h) {
                    running = false;
                    return true;
                }
                if (x >= exitButton.x && x <= exitButton.x + exitButton.w &&
                    y >= exitButton.y && y <= exitButton.y + exitButton.h) {
                    running = false;
                    return false;
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, menuTexture, NULL, NULL);

        SDL_RenderCopy(renderer, restartTexture, NULL, &restartButton);
        SDL_RenderCopy(renderer, exitTexture, NULL, &exitButton);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(restartTexture);
    SDL_DestroyTexture(exitTexture);
    TTF_CloseFont(font);

    return false;
}

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if (!init(&window, &renderer)) {
        return 1;
    }

    SDL_Texture* menuTexture = loadTexture("menu.jpg", renderer);
    if (menuTexture == NULL) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    if (!mainMenu(renderer, menuTexture)) {
        cleanUp(window, renderer, NULL, NULL, NULL, NULL, menuTexture);
        return 0;
    }

    Dinosaur dino = {NULL, {320, WINDOW_HEIGHT - GROUND_HEIGHT, 145, 150}, 0, 0, 30};
    Ghost ghost = {NULL, {0, WINDOW_HEIGHT - GROUND_HEIGHT - 80, 100, 100}, 3, true};
    SDL_Texture* treeTexture = NULL;
    SDL_Texture* cloudTexture = NULL;

    dino.texture = loadTexture("dino.png", renderer);
    if (dino.texture == NULL) {
        cleanUp(window, renderer, &dino, &ghost, treeTexture, cloudTexture, menuTexture);
        return 1;
    }

    ghost.texture = loadTexture("ghost.png", renderer);
    if (ghost.texture == NULL) {
        cleanUp(window, renderer, &dino, &ghost, treeTexture, cloudTexture, menuTexture);
        return 1;
    }

    treeTexture = loadTexture("tree.png", renderer);
    if (treeTexture == NULL) {
        cleanUp(window, renderer, &dino, &ghost, treeTexture, cloudTexture, menuTexture);
        return 1;
    }

    cloudTexture = loadTexture("cloud.png", renderer);
    if (cloudTexture == NULL) {
        cleanUp(window, renderer, &dino, &ghost, treeTexture, cloudTexture, menuTexture);
        return 1;
    }

    TTF_Font *font1 = TTF_OpenFont("arial.ttf",24);
    if(!font1){
        std::cout<<"Failed to load due to: "<<TTF_GetError()<<std::endl;
    }

    // Initialize stone positions and sizes
    Stone stones[NUM_STONES];
    for (int i = 0; i < NUM_STONES; ++i) {
        stones[i].x = rand() % WINDOW_WIDTH;
        stones[i].y = rand() % (GROUND_HEIGHT / 2) + (WINDOW_HEIGHT - GROUND_HEIGHT / 2);
        stones[i].size = rand() % 10 + 5;
    }

    bool running = true;
    int score = 0;

    while (running) {
        handleEvents(&running, &dino);
        updateDino(&dino);

        if (!ghost.active) {
            ghost.rect.x = 0;
            ghost.rect.y = WINDOW_HEIGHT - GROUND_HEIGHT - 80;
            ghost.active = true;
            ghost.velocity_x += 1;  // Increase the speed of the ghost
        }
        updateGhost(&ghost);

        if (checkCollision(&dino.rect, &ghost.rect)) {
            if (!gameOverMenu(renderer, menuTexture)) {
                running = false;
            } else {
                // Reset the game state
                dino.rect.x = 320;
                dino.rect.y = WINDOW_HEIGHT - GROUND_HEIGHT - dino.rect.h;
                ghost.rect.x = 0;
                score = 0;
            }
        }

        render(renderer, &dino, &ghost, treeTexture, cloudTexture, stones, NUM_STONES);
    }

    cleanUp(window, renderer, &dino, &ghost, treeTexture, cloudTexture, menuTexture);
    return 0;
}
