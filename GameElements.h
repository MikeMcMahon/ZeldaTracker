//
// Created by MikeMcMahon on 5/9/2016.
//

#ifndef ZELDATRACKER_GAMEELEMENTS_H
#define ZELDATRACKER_GAMEELEMENTS_H

const Uint8 SPRITE_MODA_OFF = 0x55;
const Uint8 SPRITE_MODA_HOVER = 0xAA;
const Uint8 SPRITE_MODA_ON = 0xFF;
const int SPRITE_SHEET_GRID_SIZE = 15;
const int SPRITE_STATE_OFF      = 0x0000;
const int SPRITE_STATE_ON       = 0x0001;
const int SPRITE_STATE_HOVER    = 0x0010;
const int SPRITE_STATE_DISABLED = 0x0100;

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


void GE_InitSprite(struct Sprite *sprite, int state) {
    sprite->s_x = 0;
    sprite->s_y = 0;
    sprite->x = 0;
    sprite->y = 0;
    sprite->state = state;
}

#endif //ZELDATRACKER_GAMEELEMENTS_H
