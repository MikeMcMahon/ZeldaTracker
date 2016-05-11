#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEBUG

#include "GameFonts.h"
#include "Debug.h"
#include "GameElements.h"
#include "GameMath.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_image.h"


const int WINDOW_WIDTH = 200;
const int WINDOW_HEIGHT = 720;
const char *WINDOW_TITLE = "Zelda Tracker";
const int TOTAL_SPRITES = 20;
const int SPRITE_HEIGHT = 48;
const int SPRITE_WIDTH = 48;


/********************************************//**
 * @brief Initializes the game scene.
 *
 * @param scene struct Scene*
 * @param WINDOW_WIDTH int
 * @param WINDOW_HEIGHT int
 * @return void
 *
 ***********************************************/
int ZR_InitGame(struct Scene *scene, int window_width,
                int window_height, const char *window_title) {
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

int ZR_InitGameSprites(struct Sprite *sprites) {
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
    int cur_sprite_pos = 0;
    int disabled = 0;
    int display_row = 0;
    size_t line_len = 0;
    size_t comment_len = 0;
    size_t coords_len = 0;
    while(fgets(line, sizeof(line), sprites_file)) {
        result = strchr(line, split_on);

        disabled = 0;
        if (line[0] == '0')
            disabled = 1;


        line_len = strlen(line);
        comment_len = strlen(result);
        coords_len = sizeof(char) * (line_len - comment_len);

        // Make room for the null terminator with +1
        coords = malloc(coords_len + sizeof(char));
        coords[coords_len] = '\0';
        strncpy(coords, line, coords_len);

        row[0] = '\0';
        row[1] = '\0';
        row[2] = '\0';
        col[0] = '\0';
        col[1] = '\0';
        col[2] = '\0';
        count = 0;
        get_right = 0;
        for (int i = 2; i < coords_len; i++) {
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

        GE_InitSprite(&sprites[cur_sprite], (disabled == 0) ? SPRITE_STATE_OFF : SPRITE_STATE_DISABLED);
        if (disabled == 1) {
            goto cleanup; // OH GOD A VALID USE FOR GOTO...could use a nested if/else but f. if/else branch prediction
        }

        sprites[cur_sprite].s_x = atoi(col);
        sprites[cur_sprite].s_y = atoi(row);
        if (((cur_sprite_pos - 1) % 4 == 0) && cur_sprite_pos != 1)
            display_row++;

        sprites[cur_sprite].x = 10 + (((cur_sprite_pos - 1) % 4) * SPRITE_WIDTH);
        sprites[cur_sprite].y = 376 + (display_row * SPRITE_HEIGHT) + (display_row * 16);
        cur_sprite_pos++;


        cleanup:
        cur_sprite++;
        free(coords);
    }

    fclose(sprites_file);

    return 0;
}

int ZR_InitGame(struct Scene *, int, int, const char *);
int ZR_InitGameSprites(struct Sprite *);
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

    if (ZR_InitGame(&scene, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE) != 0) {
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
    SDL_SetTextureAlphaMod(ss_texture, SPRITE_MODA_OFF);

    SDL_FreeSurface(spritesheet);

    struct Sprite game_sprites[TOTAL_SPRITES];
    if (ZR_InitGameSprites(game_sprites) != 0)
        return EXIT_FAILURE;


    SDL_Rect link_walk_frm = {
            30, 0,
            16, 16
    };
    SDL_Rect link_walk_to = {
            170,
            30,
            16,
            16
    };

    // For our custom cursor
    SDL_Rect cursor;
    cursor.h = 16;
    cursor.w = 10;
    cursor.x = 362;
    cursor.y = 195;
    SDL_Rect cursor_draw_at;
    cursor_draw_at.h = 32;
    cursor_draw_at.w = 20;

    SDL_Rect font_draw_rect;
    font_draw_rect.h = 24;
    font_draw_rect.w = 24;
    font_draw_rect.x = 10;
    font_draw_rect.y = 10;

    SDL_Rect triforce_frm =
            {
                    game_sprites[0].s_x * SPRITE_SHEET_GRID_SIZE, // x-pos in sprite sheet
                    game_sprites[0].s_y * SPRITE_SHEET_GRID_SIZE, // y-pos in sprite sheet
                    16, 16
            },
             triforce_to = {30, 0, SPRITE_WIDTH, SPRITE_HEIGHT};


    SDL_Rect items_frm = {0, 0, 16, 16},
             items_to = { 60, 120, SPRITE_WIDTH, SPRITE_HEIGHT};
    Uint8 sprite_moda_mode = SPRITE_MODA_OFF;

    //////////////////////////////
    //
    // Managing the boards/levels
    //
    //////////////////////////////
    int total_dungeons = 9;
    struct Sprite triforce_sprites[total_dungeons];
    struct Sprite dungeon_sprites[total_dungeons];

    char dungeon_display[4];
    Uint8 triforce_moda_mode = SPRITE_MODA_OFF;
    SDL_Color dungeon_color;
    dungeon_color.r = 255;
    dungeon_color.g = 255;
    dungeon_color.b = 255;
    dungeon_color.a = 255;
    SDL_Surface *dungeon_surface;
    SDL_Texture *dungeon_texture[total_dungeons];


    for (int i = 0; i < total_dungeons; i++) {
        // Handle the dungeon number surfaces
        sprintf(dungeon_display, "%d", (i + 1));
        dungeon_surface = TTF_RenderText_Blended(game_font,
                                                 dungeon_display,
                                                 dungeon_color);
        dungeon_texture[i] = SDL_CreateTextureFromSurface(scene.renderer, dungeon_surface);

        dungeon_sprites[i].x = 10;
        dungeon_sprites[i].y = 20 + (i * 40);

        triforce_sprites[i].x = 30;
        triforce_sprites[i].y = dungeon_sprites[i].y - 16;
        triforce_sprites[i].state = SPRITE_STATE_OFF;
        font_draw_rect.y = (i * 30) + 5;

        SDL_FreeSurface(dungeon_surface);
    }

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

    int mouse_pressed = 0;
    int walk_cycle = 0;
    int stabbing_running = 0;
    int facing = 0; // 0 = left, 1 = down, 2 = right, 3 = up
    float stab_ends_in = 0;
    while(quit == 0) {
        current = SDL_GetTicks();
        elapsed = current - previous;
        previous = current;
        lag += elapsed;

        mouse_pressed = 0;
        while(SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = -1;
            }

            if (e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.scancode) {

                    case SDL_SCANCODE_0:
                    case SDL_SCANCODE_1:
                    case SDL_SCANCODE_2:
                    case SDL_SCANCODE_3:
                    case SDL_SCANCODE_4:
                    case SDL_SCANCODE_5:
                    case SDL_SCANCODE_6:
                    case SDL_SCANCODE_7:
                    case SDL_SCANCODE_8:
                    case SDL_SCANCODE_9:

                    case SDL_SCANCODE_ESCAPE:
                        quit = -1;
                    default:
                        break;
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN)
                if (e.button.button & SDL_BUTTON_LEFT) {
                    mouse_pressed = 1;
            }
        }

        // This portion causes the sprite for link to change to the stabbing animation
        if (mouse_pressed == 1 && stabbing_running != 1) {
            stabbing_running = 1;
            stab_ends_in = current + (ms_per_update * 15);

            switch(facing) {
                default: // left
                    link_walk_frm.x = 24;
                    link_walk_frm.y = 90;
                    link_walk_frm.w = 28;
                    link_walk_frm.h = 16;
                    link_walk_to.x -= 12;
                    break;
                case 2: // right
                    link_walk_frm.x = 83;
                    link_walk_frm.y = 90;
                    link_walk_frm.w = 28;
                    link_walk_frm.h = 16;
                    break;
                case 3: // up
                    link_walk_frm.x = 60;
                    link_walk_frm.y = 83;
                    link_walk_frm.h = 28;
                    link_walk_frm.w = 16;
                    link_walk_to.y -= 12;
                    break;
                case 1: // down
                    link_walk_frm.x = 0;
                    link_walk_frm.y = 83;
                    link_walk_frm.h = 28;
                    link_walk_frm.w = 16;
                    break;
            }

            link_walk_to.h = link_walk_frm.h;
            link_walk_to.w = link_walk_frm.w;
        }

         // Really doubt we'll have lag, but a good habit i guess.
         SDL_GetMouseState(&cursor_draw_at.x, &cursor_draw_at.y);
         while (lag >= ms_per_update) {

             if (stab_ends_in <= current && stabbing_running == 1) {
                 stabbing_running = 0;
                 switch(facing) {
                     case 0:
                         link_walk_to.x += 12;
                         break;
                     case 3:
                         link_walk_to.y += 12;
                         break;
                     default:
                         break;
                 }
             }

             if (stabbing_running == 0) {
                link_walk_frm.y = ((current / 200) % 2) * 30;
                if (walk_cycle == 0) {
                     link_walk_to.x = (--link_walk_to.x < 80) ? 80 : link_walk_to.x;

                     if (link_walk_to.x == 80) { // time to turn facing down
                         facing = 1;
                         link_walk_frm.x = 0;
                         link_walk_to.y = (++link_walk_to.y > 348) ? 348 : link_walk_to.y;

                         if (link_walk_to.y == 348) { // time to turn facing right
                             facing = 2;
                             link_walk_frm.x = 90;
                             walk_cycle = 1;
                         }
                     }
                 }
                 if (walk_cycle == 1) {
                     link_walk_to.x = (++link_walk_to.x > 170) ? 170 : link_walk_to.x;

                     if (link_walk_to.x == 170) { // time to turn facing up
                         facing = 3;
                         link_walk_frm.x = 60;
                         link_walk_to.y = (--link_walk_to.y < 30) ? 30 : link_walk_to.y;

                         if (link_walk_to.y == 30) { // time to turn facing left
                             facing = 0;
                             link_walk_frm.x = 30;
                             walk_cycle = 0;
                         }
                     }
                 }
             }

             stab_ends_in -= ms_per_update;
             lag -= ms_per_update;
         }

        for (int i = 0; i < total_dungeons; i++) {
            triforce_to.x = triforce_sprites[i].x;
            triforce_to.y = triforce_sprites[i].y;

            if (GM_PointCollides(cursor_draw_at.x, cursor_draw_at.y, triforce_to.x,
                                 triforce_to.y, triforce_to.w, triforce_to.h) == GM_COLLIDES) {
                triforce_sprites[i].state |= SPRITE_STATE_HOVER;
                if (mouse_pressed == 1) {
                    if ((triforce_sprites[i].state & SPRITE_STATE_ON) == SPRITE_STATE_ON) {
                        triforce_sprites[i].state ^= SPRITE_STATE_ON;
                    } else {
                        triforce_sprites[i].state |= SPRITE_STATE_ON;
                    }
                }
            } else {
                if ((triforce_sprites[i].state & SPRITE_STATE_HOVER) == SPRITE_STATE_HOVER) {
                    triforce_sprites[i].state ^= SPRITE_STATE_HOVER;
                }
            }
        }


        for (int i = 1; i < TOTAL_SPRITES; i++) {
            // skip rendering / processing
            if ((game_sprites[i].state & SPRITE_STATE_DISABLED) == SPRITE_STATE_DISABLED)
                continue;

            items_to.x = game_sprites[i].x;
            items_to.y = game_sprites[i].y;

            if (GM_PointCollides(cursor_draw_at.x, cursor_draw_at.y, items_to.x,
                                 items_to.y, items_to.w, items_to.h) == GM_COLLIDES) {
                game_sprites[i].state |= SPRITE_STATE_HOVER;
                if (mouse_pressed == 1) {
                    if ((game_sprites[i].state & SPRITE_STATE_ON) == SPRITE_STATE_ON) {
                        game_sprites[i].state ^= SPRITE_STATE_ON;
                    } else {
                        game_sprites[i].state |= SPRITE_STATE_ON;
                    }
                }
            } else {
                if ((game_sprites[i].state & SPRITE_STATE_HOVER) == SPRITE_STATE_HOVER) {
                    game_sprites[i].state ^= SPRITE_STATE_HOVER;
                }
            }
        }

        SDL_RenderClear(scene.renderer);
        SDL_RenderCopy(scene.renderer, scene.texture, NULL, NULL);
        SDL_UpdateTexture(scene.texture, NULL, scene.surface->pixels, scene.surface->pitch);


        SDL_SetTextureAlphaMod(ss_texture, SPRITE_MODA_ON);
        SDL_RenderCopy(scene.renderer, ss_texture, &link_walk_frm, &link_walk_to);
        SDL_SetTextureAlphaMod(ss_texture, SPRITE_MODA_OFF);


        for (int i = 0; i < total_dungeons; i++) {
            font_draw_rect.x = dungeon_sprites[i].x;
            font_draw_rect.y = dungeon_sprites[i].y;
            triforce_to.x = triforce_sprites[i].x;
            triforce_to.y = triforce_sprites[i].y;

            triforce_moda_mode = SPRITE_MODA_OFF;
            if ((triforce_sprites[i].state & SPRITE_STATE_HOVER) == SPRITE_STATE_HOVER) {
                triforce_moda_mode = SPRITE_MODA_HOVER;
            }
            if ((triforce_sprites[i].state & SPRITE_STATE_ON) == SPRITE_STATE_ON) {
                triforce_moda_mode = SPRITE_MODA_ON;
            }

            SDL_SetTextureAlphaMod(ss_texture, triforce_moda_mode);
            SDL_RenderCopy(scene.renderer, ss_texture, &triforce_frm, &triforce_to);
            SDL_SetTextureAlphaMod(ss_texture, SPRITE_MODA_OFF);

            SDL_RenderCopy(scene.renderer, dungeon_texture[i], NULL, &font_draw_rect);
        }

        for (int i = 1; i < TOTAL_SPRITES; i++) {
            // skip rendering / processing
            if ((game_sprites[i].state & SPRITE_STATE_DISABLED) == SPRITE_STATE_DISABLED)
                continue;

            items_frm.x = game_sprites[i].s_x * 15;
            items_frm.y = game_sprites[i].s_y * 15;
            items_to.x = game_sprites[i].x;
            items_to.y = game_sprites[i].y;

            sprite_moda_mode = SPRITE_MODA_OFF;
            if ((game_sprites[i].state & SPRITE_STATE_HOVER) == SPRITE_STATE_HOVER)
                sprite_moda_mode = SPRITE_MODA_HOVER;

            if ((game_sprites[i].state & SPRITE_STATE_ON) == SPRITE_STATE_ON)
                sprite_moda_mode = SPRITE_MODA_ON;

            SDL_SetTextureAlphaMod(ss_texture, sprite_moda_mode);
            SDL_RenderCopy(scene.renderer, ss_texture, &items_frm, &items_to);
            SDL_SetTextureAlphaMod(ss_texture, SPRITE_MODA_OFF);
        }

        SDL_SetTextureAlphaMod(ss_texture, SPRITE_MODA_ON);
        SDL_RenderCopy(scene.renderer, ss_texture, &cursor, &cursor_draw_at);
        SDL_SetTextureAlphaMod(ss_texture, SPRITE_MODA_OFF);

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
