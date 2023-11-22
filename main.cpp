#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include "color.h"
#include "imageloader.h"
#include "raycaster.h"

SDL_Window* window;
SDL_Renderer* renderer;

void clear() {
    SDL_SetRenderDrawColor(renderer, 56, 56, 56, 255);
    SDL_RenderClear(renderer);
}

void draw_floor() {
    // floor color
    SDL_SetRenderDrawColor(renderer, 112, 122, 122, 255);
    SDL_Rect rect = {
            0,
            SCREEN_HEIGHT / 2,
            SCREEN_WIDTH,
            SCREEN_HEIGHT / 2
    };
    SDL_RenderFillRect(renderer, &rect);
}

void draw_ui() {
    /* int size = 256; */
    /* ImageLoader::render(renderer, "p", SCREEN_WIDTH/2.0f - size/2.0f, SCREEN_HEIGHT - size, size); */
    ImageLoader::render(renderer, "bg", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    int num_joysticks = 0;
    auto joysticks = std::vector<SDL_Joystick*>();

    if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        printf("SDL initialization failed.  Error: %s\n", SDL_GetError());
    }

    num_joysticks = SDL_NumJoysticks();
    for (int i = 0; i < num_joysticks; i++) {
        auto joystick = SDL_JoystickOpen(i);
        if (joystick == nullptr) {
            printf("Warning: unable to open game controller %d!  SDL Error: %s\n", i, SDL_GetError());
        }
        else {
            joysticks.push_back(joystick);
        }
    } // https://gist.github.com/adituv/b72ce0d0ee31e6382ae7c2136a4f7bc8

    ImageLoader::init();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    window = SDL_CreateWindow("DOOM", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    ImageLoader::loadImage("+", "../assets/wall3.png");
    ImageLoader::loadImage("-", "../assets/wall1.png");
    ImageLoader::loadImage("|", "../assets/wall2.png");
    ImageLoader::loadImage("*", "../assets/wall4.png");
    ImageLoader::loadImage("g", "../assets/wall5.png");
    /* ImageLoader::loadImage("p", "assets/player.png"); */
    ImageLoader::loadImage("bg", "../assets/background.png");
    ImageLoader::loadImage("e1", "../assets/sprite1.png");

    Raycaster r = { renderer };
    r.load_map("../assets/map.txt");

    bool running = true;
    int speed = 2;
    while(running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }

            if (event.type == SDL_JOYAXISMOTION) {
                switch(event.jaxis.axis) {
                    case 2: // Axis 2 - Left and Right
                        if (event.jaxis.value > 0) {
                            r.player.a -= M_PI / 5000000 * event.jaxis.value;
                        }
                        else if (event.jaxis.value < 0) {
                            r.player.a -= M_PI / 5000000 * event.jaxis.value;
                        }
                        break;
                    case 3: // Axis 3 - Forward and Backward
                        if (event.jaxis.value > 5000) { // Backward motion
                            r.player.x -= speed * cos(r.player.a);
                            r.player.y -= speed * sin(r.player.a);
                            std::cout << "X2 Value: " <<  r.player.x << std::endl;
                            std::cout << "Y2 Value: " <<  r.player.y << std::endl;

                        } else if (event.jaxis.value < -5000) { // Forward motion
                            r.player.x += speed * cos(r.player.a);
                            r.player.y += speed * sin(r.player.a);

                            std::cout << "X Value: " <<  r.player.x << std::endl;
                            std::cout << "Y Value: " <<  r.player.y << std::endl;
                        }
                        break;
                }
            }
        }

        clear();
        draw_floor();

        r.render();

        draw_ui();
        // render

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}