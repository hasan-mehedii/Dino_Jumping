#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <string>

using namespace std;

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
        std::cout << "SDL Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    *window = SDL_CreateWindow("Jumping Dino", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (*window == NULL) {
        std::cout << "Window Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (*renderer == NULL) {
        SDL_DestroyWindow(*window);
        std::cout << "Renderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cout << "Image Init Error: " << IMG_GetError() << std::endl;
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
        std::cout << "Image Load Texture Error: " << IMG_GetError() << std::endl;
    }
    return texture;
}

void handleEvents(bool* running, Dinosaur* dino) {
    //
}

void updateDino(Dinosaur* dino) {
    dino->rect.y += dino->velocity_y;
    dino->rect.x += dino->velocity_x;

    dino->velocity_y += GRAVITY;

    int groundLevel = WINDOW_HEIGHT - GROUND_HEIGHT - dino->rect.h;

    if (dino->rect.y >= groundLevel) {
        dino->rect.y = groundLevel;
        dino->velocity_y = 0;
        dino->jumpCount = 0;
    }

    if (dino->rect.y < 0) {
        dino->rect.y = 0;
        dino->velocity_y = 0;
    }

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
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
    SDL_Rect soilRect = {groundRect->x, groundRect->y + (groundRect->h / 2), groundRect->w, groundRect->h / 2};
    SDL_RenderFillRect(renderer, &soilRect);

    SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
    SDL_Rect grassRect = {groundRect->x, groundRect->y, groundRect->w, groundRect->h / 2};
    SDL_RenderFillRect(renderer, &grassRect);

    SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);
    for (int i = groundRect->x; i < groundRect->w; i += 10) {
        int bladeHeight = rand() % 10 + 5;
        SDL_RenderDrawLine(renderer, i, groundRect->y + (groundRect->h / 2) - bladeHeight, i, groundRect->y + (groundRect->h / 2));
    }

    SDL_SetRenderDrawColor(renderer, 105, 105, 105, 255);
    for (int i = 0; i < numStones; ++i) {
        SDL_Rect stoneRect = {stones[i].x, stones[i].y, stones[i].size, stones[i].size};
        SDL_RenderFillRect(renderer, &stoneRect);
    }
}

void renderBackground(SDL_Renderer* renderer, SDL_Texture* treeTexture, SDL_Texture* cloudTexture, Stone* stones, int numStones) {
    SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
    SDL_Rect skyRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT - GROUND_HEIGHT};
    SDL_RenderFillRect(renderer, &skyRect);

    SDL_Rect groundRect = {0, WINDOW_HEIGHT - GROUND_HEIGHT, WINDOW_WIDTH, GROUND_HEIGHT};
    renderGrassAndSoil(renderer, &groundRect, stones, numStones);

    SDL_Rect treeRect1 = {90, WINDOW_HEIGHT - GROUND_HEIGHT - 160, 180, 180};
    SDL_RenderCopy(renderer, treeTexture, NULL, &treeRect1);

    SDL_Rect treeRect2 = {750, WINDOW_HEIGHT - GROUND_HEIGHT - 190, 250, 250};
    SDL_RenderCopy(renderer, treeTexture, NULL, &treeRect2);

    SDL_Rect treeRect3 = {250, WINDOW_HEIGHT - GROUND_HEIGHT - 130, 180, 180};
    SDL_RenderCopy(renderer, treeTexture, NULL, &treeRect3);

    SDL_Rect cloudRect1 = {200, 50, 150, 100};
    SDL_RenderCopy(renderer, cloudTexture, NULL, &cloudRect1);

    SDL_Rect cloudRect2 = {400, 100, 150, 100};
    SDL_RenderCopy(renderer, cloudTexture, NULL, &cloudRect2);

    SDL_Rect cloudRect3 = {700, 50, 130, 100};
    SDL_RenderCopy(renderer, cloudTexture, NULL, &cloudRect3);
}

void render(SDL_Renderer* renderer, Dinosaur* dino, Ghost* ghost, SDL_Texture* treeTexture, SDL_Texture* cloudTexture, Stone* stones, int numStones, int score, int bestScore) {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    renderBackground(renderer, treeTexture, cloudTexture, stones, numStones);

    SDL_RenderCopy(renderer, dino->texture, NULL, &dino->rect);
    if (ghost->active) {
        SDL_RenderCopy(renderer, ghost->texture, NULL, &ghost->rect);
    }

    // Render score and best score
    TTF_Font* Sans = TTF_OpenFont("arial.ttf", 24);
    if (Sans == NULL) {
        std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
        return;
    }
    SDL_Color White = {255, 255, 255};
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, ("Score: " + std::to_string(score)).c_str(), White);
    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

    SDL_Rect Message_rect;
    Message_rect.x = 20;
    Message_rect.y = 20;
    Message_rect.w = 100;
    Message_rect.h = 50;

    SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);

    surfaceMessage = TTF_RenderText_Solid(Sans, ("Best Score: " + std::to_string(bestScore)).c_str(), White);
    Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

    Message_rect.y = 80;

    SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);

    TTF_CloseFont(Sans);

    SDL_RenderPresent(renderer);
}

int loadBestScore() {
    std::ifstream inFile("bestscore.txt");
    int bestScore = 0;
    if (inFile.is_open()) {
        inFile >> bestScore;
    }
    inFile.close();
    return bestScore;
}

void saveBestScore(int bestScore) {
    std::ofstream outFile("bestscore.txt");
    if (outFile.is_open()) {
        outFile << bestScore;
    }
    outFile.close();
}

void displayTextInput(SDL_Renderer* renderer, std::string message, std::string& inputText) {
    SDL_StartTextInput();
    SDL_Color textColor = {0, 0, 0, 255};

    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Rect inputRect = {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50, 200, 100};
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    quit = true;
                } else if (e.key.keysym.sym == SDLK_BACKSPACE && inputText.length() > 0) {
                    inputText.pop_back();
                }
            } else if (e.type == SDL_TEXTINPUT) {
                inputText += e.text.text;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        SDL_Surface* messageSurface = TTF_RenderText_Solid(font, message.c_str(), textColor);
        SDL_Texture* messageTexture = SDL_CreateTextureFromSurface(renderer, messageSurface);
        int messageWidth, messageHeight;
        SDL_QueryTexture(messageTexture, NULL, NULL, &messageWidth, &messageHeight);
        SDL_Rect messageRect = {WINDOW_WIDTH / 2 - messageWidth / 2, WINDOW_HEIGHT / 2 - 100, messageWidth, messageHeight};

        SDL_Surface* inputSurface = TTF_RenderText_Solid(font, inputText.c_str(), textColor);
        SDL_Texture* inputTexture = SDL_CreateTextureFromSurface(renderer, inputSurface);
        int inputWidth, inputHeight;
        SDL_QueryTexture(inputTexture, NULL, NULL, &inputWidth, &inputHeight);
        SDL_Rect inputRect = {WINDOW_WIDTH / 2 - inputWidth / 2, WINDOW_HEIGHT / 2, inputWidth, inputHeight};

        SDL_RenderCopy(renderer, messageTexture, NULL, &messageRect);
        SDL_RenderCopy(renderer, inputTexture, NULL, &inputRect);

        SDL_RenderPresent(renderer);

        SDL_FreeSurface(messageSurface);
        SDL_DestroyTexture(messageTexture);
        SDL_FreeSurface(inputSurface);
        SDL_DestroyTexture(inputTexture);
    }

    SDL_StopTextInput();
    TTF_CloseFont(font);
}

void getPlayerName(SDL_Renderer* renderer, std::string& playerName) {
    displayTextInput(renderer, "Enter Player Name: ", playerName);
}

void saveScore(const std::string& playerName, int score) {
    ofstream file("data.txt", ios::app);
}

int main(int argc, char *args[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!init(&window, &renderer)) {
        return 1;
    }

    std::string playerName;
    getPlayerName(renderer, playerName);

    Dinosaur dino;
    dino.texture = loadTexture("dino.png", renderer);
    dino.rect = {100, WINDOW_HEIGHT - GROUND_HEIGHT - 60, 60, 60};
    dino.velocity_y = 0;
    dino.velocity_x = 0;
    dino.jumpCount = 0;

    Ghost ghost;
    ghost.texture = loadTexture("ghost.png", renderer);
    ghost.rect = {0, 0, 60, 60};
    ghost.velocity_x = 6;
    ghost.active = false;

    SDL_Texture* treeTexture = loadTexture("tree.png", renderer);
    SDL_Texture* cloudTexture = loadTexture("cloud.png", renderer);

    Stone stones[NUM_STONES];
    for (int i = 0; i < NUM_STONES; ++i) {
        stones[i].x = rand() % WINDOW_WIDTH;
        stones[i].y = WINDOW_HEIGHT - GROUND_HEIGHT + (rand() % 40);
        stones[i].size = 5 + (rand() % 15);
    }

    bool running = true;
    int score = 0;
    int bestScore = loadBestScore();

    while (running) {
        
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    if (dino.jumpCount < MAX_JUMPS) {
                        dino.velocity_y = -JUMP_STRENGTH;
                        dino.jumpCount++;
                    }
                    break;
                case SDLK_LEFT:
                    dino.velocity_x = -MOVE_SPEED;
                    break;
                case SDLK_RIGHT:
                    dino.velocity_x = MOVE_SPEED;
                    break;
            }
        }
        if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                case SDLK_RIGHT:
                    dino.velocity_x = 0;
                    break;
            }
        }
    }

        updateDino(&dino);

        if (!ghost.active) {
            ghost.rect.x = -ghost.rect.w;
            ghost.rect.y = (rand() % (WINDOW_HEIGHT - GROUND_HEIGHT - ghost.rect.h));
            ghost.active = true;
        }
        updateGhost(&ghost);

        if (checkCollision(&dino.rect, &ghost.rect)) {
            saveScore(playerName, score);
            if (score > bestScore) {
                saveBestScore(score);
                bestScore = score;
            }
            score = 0;
            ghost.active = false;
        }

        render(renderer, &dino, &ghost, treeTexture, cloudTexture, stones, NUM_STONES, score, bestScore);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(dino.texture);
    SDL_DestroyTexture(ghost.texture);
    SDL_DestroyTexture(treeTexture);
    SDL_DestroyTexture(cloudTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
