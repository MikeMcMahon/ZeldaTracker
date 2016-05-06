#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define DEBUG

#include "GameFonts.h"
#include "Debug.h"
#include "GameMath.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_image.h"


const int window_width = 200;
const int window_height = 720;
const char *window_title = "Zelda Tracker";
const int total_sprites = 20;
const int sprite_height = 30;
const int sprite_width = 30;
const Uint8 moda_sprite_off = 0x77;
const Uint8 moda_sprite_hover = 0xCC;
const Uint8 moda_sprite_on = 0xFF;
const int sprite_on = 1;
const int sprite_off = 0;
struct Scene {
    int h;
    int w;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Surface *surface;
};

struct Sprite {
    int x_loc;
    int y_loc;
    int state;
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
    scene->w = window_width;
    scene->h = window_height;
    scene->window = SDL_CreateWindow(
             window_title,
             SDL_WINDOWPOS_CENTERED,
             SDL_WINDOWPOS_CENTERED,
             scene->w,
             scene->h,
             SDL_WINDOW_SHOWN);
    SDL_ShowCursor(0);
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

int init_game_sprites(struct Sprite sprites[total_sprites]) {
    FILE *sprites_file = fopen("sprites.cfg", "r");
    if (!sprites_file) {
        DEBUG_ERR("Unable to open the sprites configuration file");
        return EXIT_FAILURE;
    }

    char line[55];
    char *coords;
    char *result;
    char split_on = '#';
    char coords_on = ',';
    char col[3];
    char row[3];
    int count = 0;
    int get_right = 0;
    int cur_sprite = 0;
    size_t line_len = 0;
    size_t comment_len = 0;
    size_t coords_len = 0;
    col[2] = '\0';
    col[2] = '\0';
    while(fgets(line, sizeof(line), sprites_file)) {
        result = strchr(line, split_on);
        line_len = strlen(line);
        comment_len = strlen(result);
        coords_len = sizeof(char) * (line_len - comment_len);

        // Make room for the null terminator with +1
        coords = malloc(coords_len + sizeof(char));
        coords[coords_len] = '\0';
        strncpy(coords, line, coords_len);

        row[0] = '\0';
        row[1] = '\0';
        col[1] = '\0';
        col[0] = '\0';
        count = 0;
        get_right = 0;
        for (int i = 0; i < strlen(coords); i++) {
            if (coords[i] == coords_on) {
                get_right = 1;
                count = 0;
                continue;
            }

            if (get_right == 0)
                col[count] = coords[i];
            else
                row[count] = coords[i];
            count++;
        }

        sprites[cur_sprite].x_loc = atoi(col);
        sprites[cur_sprite].y_loc = atoi(row);
        sprites[cur_sprite].state = 0;
        cur_sprite++;

        free(coords);
    }

    fclose(sprites_file);

    return 0;
}

int init_game(struct Scene*, int, int, const char*);
int init_game_sprites(struct Sprite[total_sprites]);
int main (int argc, char* argv[]) {
    struct Scene scene;
    TTF_Font *game_font;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        DEBUG_ERR(SDL_GetError());
        return EXIT_FAILURE;
    }

    if (TTF_Init() != 0) {
        DEBUG_ERR(TTF_GetError());
        return EXIT_FAILURE;
    }

    IMG_Init(IMG_INIT_PNG);
    game_font = TTF_OpenFont(GF_PRESS_START2P, 16);

    if (init_game(&scene, window_width, window_height, window_title) != 0) {
        return EXIT_FAILURE;
    }

    // Setup the system
    SDL_SetRenderDrawColor(scene.renderer, 0, 0, 0, 255);

    //////////////////////////////
    //
    // Sprites / buttons for clicking
    //
    //////////////////////////////
    SDL_Surface *spritesheet = IMG_Load("sprites-link.png");
    SDL_Texture *ss_texture = SDL_CreateTextureFromSurface(scene.renderer,
                                                           spritesheet);
    SDL_SetTextureBlendMode(ss_texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(ss_texture, moda_sprite_off);

    SDL_FreeSurface(spritesheet);
    SDL_Surface *sprite_highlighter = SDL_CreateRGBSurface(0, 1, 1, 32,
                                                           0, 0, 0, 0);
    SDL_FillRect(sprite_highlighter, NULL, SDL_MapRGB(sprite_highlighter->format,
                                                      255, 255, 255));
    SDL_Texture *sh_texture = SDL_CreateTextureFromSurface(scene.renderer,
                                                           sprite_highlighter);
    SDL_SetTextureBlendMode(sh_texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(sh_texture, 0x99);

    struct Sprite game_sprites[total_sprites];
    if (init_game_sprites(game_sprites) != 0)
        return EXIT_FAILURE;


    SDL_Rect cpy_from;
    cpy_from.h = 16;
    cpy_from.w = 16;
    cpy_from.x = 0;
    cpy_from.y = 0;

    SDL_Rect cpy_to;
    cpy_to.h = 30;
    cpy_to.w = 30;
    cpy_to.x = 30;
    cpy_to.y = 30;

    // For our custom cursor
    SDL_Rect cursor;
    cursor.h = 16;
    cursor.w = 10;
    cursor.x = 362;
    cursor.y = 195;
    SDL_Rect cursor_draw_at;
    cursor_draw_at.h = 32;
    cursor_draw_at.w = 20;

    //////////////////////////////
    //
    // Shit for the sprites
    //
    //////////////////////////////
    SDL_Rect draw_rect;
    SDL_Rect triforce_frm = {game_sprites[0].x_loc * 15, game_sprites[0].y_loc * 15, 15, 15},
             triforce_to = {60, 60, sprite_width, sprite_height};

    SDL_Rect items_frm = {0, 0, 16, 16},
             items_to = { 60, 120, sprite_width, sprite_height};
    Uint8 sprite_moda_mode = moda_sprite_off;

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
    SDL_Texture *dungeon_texture[total_dungeons];

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


    for (int i = 0; i < total_dungeons; i++) {
        SDL_GetMouseState(&cursor_draw_at.x, &cursor_draw_at.y);

        // Handle the dungeon number surfaces
        sprintf(dungeon_display, "%d", i);
        dungeon_surface = TTF_RenderText_Blended(game_font,
                                                 dungeon_display,
                                                 dungeon_color);
        dungeon_texture[i] = SDL_CreateTextureFromSurface(scene.renderer, dungeon_surface);

        draw_rect.y = (i * 30) + 5;

        SDL_FreeSurface(dungeon_surface);
    }

    int mouse_pressed = 0;
    float next_mouse_check = 0;
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
        if (e.type == SDL_MOUSEBUTTONUP)
            if (e.button.button & SDL_BUTTON_LEFT && e.button.clicks == 1) {
                mouse_pressed = 1;
                printf("mouse button was pressed/released\n");
            }
        else
                mouse_pressed = 0;

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
            SDL_GetMouseState(&cursor_draw_at.x, &cursor_draw_at.y);
            draw_rect.y = (i * 30) + 5;
            SDL_RenderCopy(scene.renderer, dungeon_texture[i], NULL, &draw_rect);
        }

        SDL_UpdateTexture(scene.texture, NULL,
                          scene.surface->pixels, scene.surface->pitch);


        // This portion renders the "buttons"
        if (point_collides(cursor_draw_at.x, cursor_draw_at.y, cpy_to.x, cpy_to.y, cpy_to.w, cpy_to.h) == 1)
            SDL_RenderCopy(scene.renderer, sh_texture, NULL, &cpy_to);

        SDL_RenderCopy(scene.renderer, ss_texture, &cpy_from, &cpy_to);
//        SDL_RenderCopy(scene.renderer, ss_texture, &triforce_frm, &triforce_to);

        for (int i = 1; i < total_sprites; i++) {
            items_frm.x = game_sprites[i].x_loc * 15;
            items_frm.y = game_sprites[i].y_loc * 15;
            items_to.y = 120 + ((i - 1) * 30);

            if (point_collides(cursor_draw_at.x, cursor_draw_at.y, items_to.x, items_to.y, items_to.w, items_to.h) == 1) {
                if (mouse_pressed == 1) {
                    if (game_sprites[i].state == sprite_on) {
                        game_sprites[i].state = sprite_off;
                        printf("STATE IS ON\n");
                    }
                    else {
                        game_sprites[i].state = sprite_on;
                        printf("STATE IS OFF\n");
                    }
                }

                sprite_moda_mode = moda_sprite_hover;
            }

            if (game_sprites[i].state == sprite_on)
                sprite_moda_mode = moda_sprite_on;

            SDL_SetTextureAlphaMod(ss_texture, sprite_moda_mode);
            SDL_RenderCopy(scene.renderer, ss_texture, &items_frm, &items_to);
            sprite_moda_mode = moda_sprite_off;
            SDL_SetTextureAlphaMod(ss_texture, sprite_moda_mode);

        }
        mouse_pressed = 0;

        SDL_SetTextureAlphaMod(ss_texture, moda_sprite_on);
        SDL_RenderCopy(scene.renderer, ss_texture, &cursor, &cursor_draw_at);
        SDL_SetTextureAlphaMod(ss_texture, moda_sprite_off);

        SDL_RenderPresent(scene.renderer);
    }

    /////////////////////
    //
    // ALWAYS CLEANUP ASSETS DUDE
    //
    /////////////////////
    for (int i = 0; i < total_dungeons; i++)
        SDL_DestroyTexture(dungeon_texture[i]);

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
