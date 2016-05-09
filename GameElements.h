//
// Created by MikeMcMahon on 5/9/2016.
//

#ifndef ZELDATRACKER_GAMEELEMENTS_H
#define ZELDATRACKER_GAMEELEMENTS_H
#include "SDL2/SDL.h"

struct Scene {
    int h;
    int w;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Surface *surface;
};

struct Sprite {
    int s_x;  // x column location in sprite sheet
    int s_y;  // y column location in sprite sheet
    int x;      // real world location for rendering
    int y;      // real world location for rendering
    int state;
};

#endif //ZELDATRACKER_GAMEELEMENTS_H
