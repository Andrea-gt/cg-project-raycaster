#pragma once
#include <iostream>
#include <fstream>
#include <SDL2/SDL_render.h>
#include <string>
#include <vector>
#include <cmath>
#include <SDL2/SDL.h>
#include <unordered_map>
#include "color.h"
#include "imageloader.h"


const Color B = {0, 0, 0};
const Color W = {255, 255, 255};

const int WIDTH = 16;
const int HEIGHT = 11;
const int BLOCK = 50;
const int SCREEN_WIDTH = WIDTH * BLOCK;
const int SCREEN_HEIGHT = HEIGHT * BLOCK;


struct Player {
  int x;
  int y;
  float a;
  float fov;
}; 

struct Impact {
  float d;
  std::string mapHit;  // + | -
  int tx;
};

class Raycaster {
public:
  Raycaster(SDL_Renderer* renderer)
    : renderer(renderer) {

    player.x = BLOCK + BLOCK / 2;
    player.y = BLOCK + BLOCK / 2;

    player.a = M_PI / 4.0f;
    player.fov = M_PI / 3.0f;

    scale = 50;
    tsize = 128;

  }

    void simulateSnow() {
        // Initialize snowflakes if it's the first frame
        if (snowflakes.empty()) {
            for (int i = 0; i < numSnowflakes; ++i) {
                int snowX = rand() % SCREEN_WIDTH;
                int snowY = rand() % SCREEN_HEIGHT;
                snowflakes.push_back({ snowX, snowY });
            }
        }

        // Change position of snowflakes every few frames
        if (frameCount % framesPerUpdate == 0) {
            for (auto& flake : snowflakes) {
                int moveX = rand() % 10 - 1; // Random movement in X direction
                int moveY = rand() % 10 - 1; // Random movement in Y direction

                // Check boundaries before moving
                int newX = flake.first + moveX;
                int newY = flake.second + moveY;

                if (newX >= 0 && newX < SCREEN_WIDTH && newY >= 0 && newY < SCREEN_HEIGHT) {
                    flake.first = newX;
                    flake.second = newY;
                } else if (newX >= 0 && newX > SCREEN_WIDTH) {
                    flake.first = 0;
                    flake.second = newY;
                } else {
                    flake.first = newX;
                    flake.second = 0;
                }
            }
        }

        // Draw snowflakes
        for (const auto& flake : snowflakes) {
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    int nx = flake.first + dx;
                    int ny = flake.second + dy;

                    if (nx >= 0 && nx < SCREEN_WIDTH && ny >= 0 && ny < SCREEN_HEIGHT) {
                        Uint8 alpha = rand() % 50 + 128;
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
                        SDL_RenderDrawPoint(renderer, nx, ny);
                    }
                }
            }
        }

        frameCount++;
    }

  void load_map(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (getline(file, line)) {
      map.push_back(line);
    }
    file.close();
  }

  void point(int x, int y, Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderDrawPoint(renderer, x, y);
  }

    void drawPixel(int x, int y, const std::string& mapHit, double h, double f) {
        int tx = static_cast<int>((h * tsize));
        int ty = static_cast<int>((f * tsize));

        Color c = ImageLoader::getPixelColor(mapHit, tx, ty);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
        SDL_RenderDrawPoint(renderer, x, y);
    }

  Impact cast_ray(float a) {
    float d = 0;
    std::string mapHit;
    int tx;

    while(true) {
      int x = static_cast<int>(player.x + d * cos(a)); 
      int y = static_cast<int>(player.y + d * sin(a)); 
      
      int i = static_cast<int>(x / BLOCK);
      int j = static_cast<int>(y / BLOCK);


      if (map[j][i] != ' ') {
        mapHit = map[j][i];

        int hitx = x - i * BLOCK;
        int hity = y - j * BLOCK;
        int maxhit;

        if (hitx == 0 || hitx == BLOCK - 1) {
          maxhit = hity;
        } else {
          maxhit = hitx;
        }

        tx = maxhit * tsize / BLOCK;

        break;
      }
     
      /* point(x, y, W); */
      
      d += 1;
    }
    return Impact{d, mapHit, tx};
  }

  void draw_stake(int x, float h, Impact i, float fogFactor) {
    float start = SCREEN_HEIGHT/2.0f - h/2.0f;
    float end = start + h;

    for (int y = start; y < end; y++) {
      int ty = (y - start) * tsize / h;
      Color wallColor = ImageLoader::getPixelColor(i.mapHit, i.tx, ty);
      Color fogColor(108, 99, 116); // Fog color

      // Interpolate between the wall color and fog color based on fog factor
      Color c = wallColor * (1.0f - fogFactor) + fogColor * fogFactor ;

      SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
      SDL_RenderDrawPoint(renderer, x, y);
    }
  } 
 
  void render() {

      // Fog variables
      const float maxRenderDistance = 500.0f; // Adjust this as needed for your scene

    // draw left side of the screen
   /* 
    for (int x = 0; x < SCREEN_WIDTH; x += BLOCK) {
      for (int y = 0; y < SCREEN_HEIGHT; y += BLOCK) {
        int i = static_cast<int>(x / BLOCK);
        int j = static_cast<int>(y / BLOCK);
        
        if (map[j][i] != ' ') {
          std::string mapHit;
          mapHit = map[j][i];
          Color c = Color(255, 0, 0);
          rect(x, y, mapHit);
        }
      }
    }

    for (int i = 1; i < SCREEN_WIDTH; i++) {
      float a = player.a + player.fov / 2 - player.fov * i / SCREEN_WIDTH;
      cast_ray(a);
    }
*/
    // draw right side of the screen

    for (int i = 0; i < SCREEN_WIDTH; i++) {
      double a = player.a + player.fov / 2.0 - player.fov * i / SCREEN_WIDTH;
      Impact impact = cast_ray(a);
      float d = impact.d;

      // Calculate fog effect based on distance
      float fogFactor = std::min(d / maxRenderDistance, 1.0f);

      if (d == 0) {
        exit(1);
      }
      int x = i;
      float h = static_cast<float>(SCREEN_HEIGHT)/static_cast<float>(d * cos(a - player.a)) * static_cast<float>(scale);
      draw_stake(x, h, impact, fogFactor);
    }

    simulateSnow();

      int xD =player.x-3*BLOCK/2;
      int yD =player.y-3*BLOCK/2;
      int sizeX = map[0].size();
      int sizeY = map.size();

      for (int x = 0; x < 4 * BLOCK; x += 1) {
          for (int y = 0; y <4 * BLOCK; y += 1) {
              double i = 1.0* (xD + x) / BLOCK;
              double j = 1.0* (yD + y) / BLOCK;

              if (i >= sizeX || j >= sizeY)
                  continue;

              int i_floor = static_cast<int>(i);
              int j_floor = static_cast<int>(j);
              double h = i - i_floor;
              double f = j - j_floor;

              if (map[j_floor][i_floor] != ' ') {
                  std::string mapHit;
                  mapHit = map[j_floor][i_floor];
                  drawPixel(x, y, mapHit, h, f);
              } else {
                  point(x,y ,B);
              }
          }
      }
  }

  Player player;
private:
  int scale;
  SDL_Renderer* renderer;
  std::vector<std::string> map;
  int tsize;
  int frameCount = 0;
  const int framesPerUpdate = 2;
  const int numSnowflakes = 100;
  std::vector<std::pair<int, int>> snowflakes; // Store positions of snowflakes
};
