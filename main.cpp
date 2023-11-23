#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_mixer.h>
#include "color.h"
#include "imageloader.h"
#include "raycaster.h"
#include <sstream>

// Music
#define BG_PATH "../assets/bgMusic.wav"

// Sound effects
#define WAV_PATH "../assets/footSteps.wav"

//The music that will be played
Mix_Music* bgMusic = NULL;

//The sound effects that will be used
Mix_Chunk* gFootsteps = NULL;

//Player sprite
std::string playerStatus= "DEFAULT";

bool isMenu = true;

SDL_Window* window;
SDL_Renderer* renderer;

void clear() {
    SDL_SetRenderDrawColor(renderer, 108, 99, 116, 255);
    SDL_RenderClear(renderer);
}

void draw_floor() {
    // floor color
    SDL_SetRenderDrawColor(renderer, 28, 38, 41, 255);
    SDL_Rect rect = {
            0,
            SCREEN_HEIGHT / 2,
            SCREEN_WIDTH,
            SCREEN_HEIGHT / 2
    };
    SDL_RenderFillRect(renderer, &rect);
}

void draw_ui() {
    // Player sprite
    ImageLoader::render(renderer, playerStatus, SCREEN_WIDTH/2.0f - 200/2.0f + 100, SCREEN_HEIGHT - 147, 200, 147);
    // Inside the main loop after initializing SDL and other components
    ImageLoader::render(renderer, "bg", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 40);

    if(isMenu){
        ImageLoader::render(renderer, "menu", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 100);
    }
  }

  int main() {
      Uint32 frameStart, frameTime;
      std::string title = "FPS: ";

      SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
      SDL_SetRelativeMouseMode(SDL_TRUE);

      const int VOLUME_LEVEL = 20; // Adjust this as needed
      const int STEPS_FOR_SOUND = 10;
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


      //Initialize SDL_mixer
      if(Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
      {
          printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
      }

      //Load sound effects
      bgMusic = Mix_LoadMUS( BG_PATH);
      if( bgMusic == NULL )
      {
          printf( "Failed to load music! SDL_mixer Error: %s\n", Mix_GetError() );
      }

      //Load sound effects
      gFootsteps = Mix_LoadWAV(WAV_PATH);
      if( gFootsteps == NULL )
      {
          printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
      }

      ImageLoader::init();
      SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

      window = SDL_CreateWindow("DOOM", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

      ImageLoader::loadImage("+", "../sh1-textures/wall5.png");
      ImageLoader::loadImage("-", "../sh1-textures/wall1.png");
      ImageLoader::loadImage("|", "../sh1-textures/wall2.png");
      ImageLoader::loadImage("*", "../sh1-textures/wall4.png");
      ImageLoader::loadImage("g", "../sh1-textures/wall3.png");
      ImageLoader::loadImage("DEFAULT", "../assets/pistol.png");
      ImageLoader::loadImage("LEFT", "../assets/pistolLeft.png");
      ImageLoader::loadImage("RIGHT", "../assets/pistolRight.png");
      ImageLoader::loadImage("bg", "../assets/fog.png");
      ImageLoader::loadImage("menu", "../sh1-textures/titleScreen.png");

      Raycaster r = { renderer };
      r.load_map("../assets/map.txt");

      bool running = true;
      int speed = 2;
      bool isMoving = false; // Flag to track if the player is moving
      int isStep = 0; // Flag to track if the player is moving
      float oldX = r.player.x;
      float oldY = r.player.y;
      // Play background music
      Mix_PlayMusic(bgMusic, -1); // -1 loops the music indefinitely

      while(running) {
          SDL_Event event;
          frameStart = SDL_GetTicks();

          while (SDL_PollEvent(&event)) {
              if (event.type == SDL_QUIT) {
                  running = false;
                  break;
              }

              if (event.type == SDL_JOYAXISMOTION) {
                  switch(event.jaxis.axis) {
                      case 2: // Axis 2 - Left and Right
                          if (event.jaxis.value > 0 && isMenu == false) {
                              playerStatus= "LEFT";
                              r.player.a -= M_PI / 5000000 * event.jaxis.value;
                          }
                          else if (event.jaxis.value < 0 && isMenu == false) {
                              playerStatus= "RIGHT";
                              r.player.a -= M_PI / 5000000 * event.jaxis.value;
                          }
                          break;
                      case 3: // Axis 3 - Forward and Backward
                          if (event.jaxis.value > 1000 && isMenu == false) { // Backward motion
                              r.player.x -= speed * cos(r.player.a);
                              r.player.y -= speed * sin(r.player.a);
                              isMoving = true; // Player is moving
                              isStep++;
                          } else if (event.jaxis.value < -1000 && isMenu == false) { // Forward motion
                              r.player.x += speed * cos(r.player.a);
                              r.player.y += speed * sin(r.player.a);
                              isMoving = true; // Player is moving
                              isStep++;
                          } else {
                              playerStatus = "DEFAULT";
                              isMoving = false; // Player stopped moving
                          }
                          break;
                  }
              }

              else if (event.type == SDL_KEYDOWN) {
                  switch(event.key.keysym.sym ){
                      case SDLK_LEFT:
                          if(isMenu==false){
                              r.player.a += 3.14/24;
                              playerStatus= "LEFT";
                          }
                          break;
                      case SDLK_RIGHT:
                          if(isMenu==false) {
                              playerStatus = "RIGHT";
                              r.player.a -= 3.14 / 24;
                          }
                          break;
                      case SDLK_UP:
                          if(isMenu==false) {
                              r.player.x += speed * cos(r.player.a);
                              r.player.y += speed * sin(r.player.a);
                              isMoving = true; // Player is moving
                              isStep++;
                          }
                          break;
                      case SDLK_DOWN:
                          if(isMenu==false) {
                              r.player.x -= speed * cos(r.player.a);
                              r.player.y -= speed * sin(r.player.a);
                              isMoving = true; // Player is moving
                              isStep++;
                          }
                          break;
                      case SDLK_ESCAPE:
                          running = false;
                          break;
                      case SDLK_RETURN:
                          isMenu = false;
                          break;
                  }
              }

              // Check if any arrow key was released to stop the movement
              else if (event.type == SDL_KEYUP) {
                  switch(event.key.keysym.sym) {
                      case SDLK_LEFT:
                      case SDLK_RIGHT:
                      case SDLK_UP:
                      case SDLK_DOWN:
                          playerStatus = "DEFAULT";
                          isMoving = false; // Player stopped moving
                          break;
                  }
              }

              else if (event.type == SDL_MOUSEMOTION) {
                  if (event.motion.xrel < 0) {
                      r.player.a += M_PI / 1000;
                      playerStatus = "LEFT";
                  } else if (event.motion.xrel > 0) {
                      r.player.a -= M_PI / 1000;
                      playerStatus = "RIGHT";
                  } else {
                      playerStatus = "DEFAULT"; // Set default when no movement detected
                  }
              }

              // Play footsteps sound only if the player is moving
              if (isMoving && isStep == STEPS_FOR_SOUND) {
                  Mix_VolumeChunk(gFootsteps, VOLUME_LEVEL); // Set the volume of the sound
                  Mix_PlayChannel(-1, gFootsteps, 0);
                  isStep = 0;
              }
          }

          clear();
          draw_floor();

          if(!r.render()){
              r.player.x = oldX;
              r.player.y = oldY;
              r.render();
          };

          draw_ui();
          // render

          SDL_RenderPresent(renderer);

          // Calculate frames per second and update window title
          frameTime = SDL_GetTicks() - frameStart;
          if (frameTime > 0) {
              std::ostringstream titleStream;
              titleStream << "FPS: " << 1000.0 / frameTime;  // Milliseconds to seconds
              SDL_SetWindowTitle(window, titleStream.str().c_str());
          }

          oldX = r.player.x;
          oldY = r.player.y;

      }

      SDL_DestroyWindow(window);
      SDL_Quit();
  }