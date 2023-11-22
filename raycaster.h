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

struct Enemy {
  int x;
  int y;
  std::string texture;
};

std::vector<Enemy> enemies;

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

    enemies = {Enemy{BLOCK * 5, BLOCK, "e1"}};
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

  void rect(int x, int y, const std::string& mapHit) {
    for(int cx = x; cx < x + BLOCK; cx++) {
      for(int cy = y; cy < y + BLOCK; cy++) {
        int tx = ((cx - x) * tsize) / BLOCK;
        int ty = ((cy - y) * tsize) / BLOCK;

        Color c = ImageLoader::getPixelColor(mapHit, tx, ty);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b , 255);
        SDL_RenderDrawPoint(renderer, cx, cy);
      }
    }
  }

  void draw_enemy(Enemy enemy) {
    float enemy_a = atan2(enemy.y - player.y, enemy.x - player.x);
    float enemy_d = sqrt(pow(player.x - enemy.x, 2) + pow(player.y - enemy.y, 2)); 
    int enemy_size = (SCREEN_HEIGHT/enemy_d) * scale;

    int enemy_x = (enemy_a - player.a) * (SCREEN_WIDTH / player.fov) + SCREEN_WIDTH / 2.0f - enemy_size / 2.0f;
    int enemy_y = (SCREEN_HEIGHT / 2.0f) - enemy_size / 2.0f;

    for(int x = enemy_x; x < enemy_x + enemy_size; x++) {
      for(int y = enemy_y; y < enemy_y + enemy_size; y++) {
        int tx = (x - enemy_x) * tsize / enemy_size;
        int ty = (y - enemy_y) * tsize / enemy_size;

        Color c = ImageLoader::getPixelColor(enemy.texture, tx, ty);
        if (c.r != 152 && c.g != 0 && c.b != 136) {
          SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
          SDL_RenderDrawPoint(renderer, x, y);
        }
      }
    }
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

  void draw_stake(int x, float h, Impact i) {
    float start = SCREEN_HEIGHT/2.0f - h/2.0f;
    float end = start + h;

    for (int y = start; y < end; y++) {
      int ty = (y - start) * tsize / h;
      Color c = ImageLoader::getPixelColor(i.mapHit, i.tx, ty);
      SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

      SDL_RenderDrawPoint(renderer, x, y);
    }
  } 
 
  void render() {
    
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
      Color c = Color(255, 0, 0);

      if (d == 0) {
        exit(1);
      }
      int x = i;
      float h = static_cast<float>(SCREEN_HEIGHT)/static_cast<float>(d * cos(a - player.a)) * static_cast<float>(scale);
      draw_stake(x, h, impact);
    }

    for(Enemy enemy : enemies) {
      draw_enemy(enemy);
    }

  }

  Player player;
private:
  int scale;
  SDL_Renderer* renderer;
  std::vector<std::string> map;
  int tsize;
};
