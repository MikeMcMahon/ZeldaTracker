#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define DEBUG

#include "GameFonts.h"
#include "Debug.h"
#include "GameManager.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_image.h"


const int window_width = 140;
const int window_height = 720;
const char *window_title = "Zelda Tracker";

struct Scene {
    int h;
    int w;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Surface *surface;
};


/********************************************//**
 * @brief Initializes the game scene.
 *
 * @param scene struct Scene*
 * @param window_width int
 * @param window_height int
 * @return void
 *
 ***********************************************/
int init_game(struct Scene *scene, int window_width,
               int window_height, const char* window_title) {
    DEBUG_LOG("Initializing the Scene/Window");
    SDL_LogInfo(SDL_LOG_PRIORITY_INFO, "No worries");
    scene->w = window_width;
    scene->h = window_height;
    scene->window = SDL_CreateWindow(
             window_title,
             SDL_WINDOWPOS_CENTERED,
             SDL_WINDOWPOS_CENTERED,
             scene->w,
             scene->h,
             SDL_WINDOW_SHOWN);
    if (scene->window == NULL) {
            DEBUG_ERR(SDL_GetError());
            return -1;
    }

    scene->renderer = SDL_CreateRenderer(
                                scene->window,
                                -1,
                                SDL_RENDERER_ACCELERATED);
    if (scene->window == NULL) {
            DEBUG_ERR(SDL_GetError());
            return -1;
    }

    scene->surface = SDL_GetWindowSurface(scene->window);
    if (scene->surface == NULL) {
            DEBUG_ERR(SDL_GetError());
            return -1;
    }

    scene->texture = SDL_CreateTextureFromSurface(
                                         scene->renderer,
                                         scene->surface);
    if (scene->texture == NULL) {
            DEBUG_ERR(SDL_GetError());
            return -1;
    }

    return 0;
}


int init_game(struct Scene*, int, int, const char*);
int main (int argc, char* argv[]) {
    struct Scene scene;
    TTF_Font *game_font;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        DEBUG_ERR(SDL_GetError());
        return -1;
    }

    if (TTF_Init() != 0) {
        DEBUG_ERR(TTF_GetError());
        return -1;
    }

    IMG_Init(IMG_INIT_PNG);
    game_font = TTF_OpenFont(GF_PRESS_START2P, 16);

    if (init_game(&scene, window_width, window_height, window_title) != 0) {
        return -1;
    }
    SDL_SetRenderDrawColor(scene.renderer, 0, 0, 0, 255);

    //////////////////////////////
    //
    // Sprites / buttons for clicking
    //
    //////////////////////////////
    SDL_Surface *spritesheet = IMG_Load("sprites-link.gif");
    SDL_Texture *ss_texture = SDL_CreateTextureFromSurface(scene.renderer,
                                                           spritesheet);
    SDL_Rect cpy_from;
    cpy_from.h = 30;
    cpy_from.w = 30;
    cpy_from.x = 0;
    cpy_from.y = 0;

    SDL_Rect cpy_to;
    cpy_to.h = 30;
    cpy_to.w = 30;
    cpy_to.x = 30;
    cpy_to.y = 30;

    //////////////////////////////
    //
    // Drawing rectangle for drawing
    //
    //////////////////////////////
    SDL_Rect draw_rect;

    //////////////////////////////
    //
    // Managing the boards/levels
    //
    //////////////////////////////
    int total_dungeons = 10;
    char dungeon_display[4];
    SDL_Color dungeon_color;
    dungeon_color.r = 255;
    dungeon_color.g = 255;
    dungeon_color.b = 255;
    dungeon_color.a = 255;
    SDL_Surface *dungeon_surface;
    SDL_Texture *dungeon_texture;

    //////////////////////////////
    //
    // For handling the game loop
    //
    //////////////////////////////
    unsigned int previous;
    unsigned int current = 0;
    float lag = 0;
    unsigned int elapsed = 0;
    float frames_per_second = 60;
    float ms_per_update = (1000 / frames_per_second);
    SDL_Event e;
    int quit = 0;

    previous = SDL_GetTicks();
    while(quit == 0) {
        current = SDL_GetTicks();
        elapsed = current - previous;
        previous = current;
        lag += elapsed;

        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT)
            quit = -1;
        if (e.type == SDL_KEYDOWN)
            if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                quit = -1;

         // Really doubt we'll have lag, but a good habit i guess.
         while (lag >= ms_per_update) {
            cpy_from.x = ((SDL_GetTicks() / 200) % 4) * 30;
            lag -= ms_per_update;
         }

        SDL_RenderClear(scene.renderer);
        SDL_RenderCopy(scene.renderer, scene.texture, NULL, NULL);

        // This renders the *game board* levels
        draw_rect.h = 15;
        draw_rect.w = 15;
        draw_rect.x = 10;
        draw_rect.y = 10;
        for (int i = 0; i < total_dungeons; i++) {
            sprintf(dungeon_display, "%d", i);
            dungeon_surface = TTF_RenderText_Blended(game_font,
                                                     dungeon_display,
                                                     dungeon_color);
            dungeon_texture = SDL_CreateTextureFromSurface(scene.renderer, dungeon_surface);

            draw_rect.y = (i * 25) + 5;
            SDL_RenderCopy(scene.renderer, dungeon_texture, NULL, &draw_rect);
            SDL_DestroyTexture(dungeon_texture);
        }

        SDL_UpdateTexture(scene.texture, NULL,
                          scene.surface->pixels, scene.surface->pitch);


        // This portion renders the "buttons"
        SDL_RenderCopy(scene.renderer, ss_texture, &cpy_from, &cpy_to);
        SDL_RenderPresent(scene.renderer);
    }

    SDL_FreeSurface(scene.surface);
    SDL_DestroyTexture(ss_texture);
    SDL_DestroyTexture(scene.texture);
    SDL_DestroyRenderer(scene.renderer);
    SDL_DestroyWindow(scene.window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
