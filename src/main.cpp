#include <thread>
#include <map>

#include "SDL2/SDL.h"
#include "SDL2/SDL_timer.h"
#include "chip8.hpp"

auto epoch = std::chrono::high_resolution_clock::from_time_t(0);

auto now() {
    return std::chrono::high_resolution_clock::now();
}

auto millis() {
    return std::chrono::duration_cast
    <std::chrono::milliseconds>(now() - epoch).count();
}

typedef std::pair<SDL_Scancode, uint8_t> gameKey;
std::map<gameKey::first_type, gameKey::second_type> gameKeys;

void loop(SDL_Event event, SDL_Renderer* renderer, SDL_Texture* texture, Chip8& chip8) {
    auto last = millis();
    bool run = true;
    while (run) {
        chip8.emulateCycle();

        if (millis() - last >= 1000/60) {
            last = millis();

            if (chip8.delay_timer > 0) {
                --chip8.delay_timer;
            }
            if (chip8.sound_timer > 0) {
                if (chip8.sound_timer == 1) {
                    // printf("BEEP");
                    ;
                }
                --chip8.sound_timer;
            }
            const uint8_t *pressedKeys = SDL_GetKeyboardState(NULL);
            for (auto const& [key, value] : gameKeys) {
                chip8.key[value] = pressedKeys[key];
            }

            chip8.key[0x10] = 0x10;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        run = false;
                        break;

                    case SDL_KEYDOWN:
                        if (gameKeys.count(event.key.keysym.scancode)) {
                            chip8.key[0x10] = gameKeys[event.key.keysym.scancode];
                        }
                        break;

                    case (SDL_DROPFILE):
                        char* path = event.drop.file;
                        chip8.init();
                        chip8.load(path);
                        break;
                }
            }
            SDL_UpdateTexture(texture, NULL, chip8.gfx, 64 * sizeof(uint8_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }
        SDL_Delay(1);
    }
}

int main() {

    Chip8 chip8;
    chip8.init();

    const int WIDTH = 640;
    const int HEIGHT = 320;
    const float SCALE = 10.0f;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

    SDL_Event event;

    SDL_Window *window = SDL_CreateWindow("CHIP-8",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WIDTH, HEIGHT, 0);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_RenderSetScale(renderer, SCALE, SCALE);

    SDL_Texture *texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_TARGET, 64, 32);

    SDL_UpdateTexture(texture, NULL, chip8.gfx, 64 * sizeof(uint8_t));

    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

    gameKeys.insert(gameKey(SDL_SCANCODE_X, 0x0));
    gameKeys.insert(gameKey(SDL_SCANCODE_1, 0x1));
    gameKeys.insert(gameKey(SDL_SCANCODE_2, 0x2));
    gameKeys.insert(gameKey(SDL_SCANCODE_3, 0x3));
    gameKeys.insert(gameKey(SDL_SCANCODE_Q, 0x4));
    gameKeys.insert(gameKey(SDL_SCANCODE_W, 0x5));
    gameKeys.insert(gameKey(SDL_SCANCODE_E, 0x6));
    gameKeys.insert(gameKey(SDL_SCANCODE_A, 0x7));
    gameKeys.insert(gameKey(SDL_SCANCODE_S, 0x8));
    gameKeys.insert(gameKey(SDL_SCANCODE_D, 0x9));
    gameKeys.insert(gameKey(SDL_SCANCODE_Z, 0xA));
    gameKeys.insert(gameKey(SDL_SCANCODE_C, 0xB));
    gameKeys.insert(gameKey(SDL_SCANCODE_4, 0xC));
    gameKeys.insert(gameKey(SDL_SCANCODE_R, 0xD));
    gameKeys.insert(gameKey(SDL_SCANCODE_F, 0xE));
    gameKeys.insert(gameKey(SDL_SCANCODE_V, 0xF));

    SDL_bool done = SDL_FALSE;
    while (!done) {
        while (!done && SDL_PollEvent(&event)) {
            switch (event.type) {
                case (SDL_QUIT): {
                    done = SDL_TRUE;
                    break;
                }

                case (SDL_DROPFILE): {
                    char* path = event.drop.file;
                    chip8.load(path);
                    loop(event, renderer, texture, chip8);
                    done = SDL_TRUE;
                    break;
               }
            }
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        }
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
