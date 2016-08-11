#ifndef __CHIP8GRAPHICS_H__
#define __CHIP8GRAPHICS_H__

#include <SDL.h>
#include "chip8.h"

typedef struct chip8graphics {
        SDL_Window *window;
        /* area to draw pixels on */
        SDL_Surface *screen_surface;
        /* 64*32 squares which represent state of each pixel*/
        SDL_Rect squares[64 * 32];
        /* how big squares on screen are. deafault value: 10 pixels */
        int square_side;
} C8graphics;

void chip8graphics_init(C8graphics*);

void chip8graphics_destroy(C8graphics*);

void chip8graphics_draw(C8graphics*, C8*);

#endif /* __CHIP8GRAPHICS_H__ */
