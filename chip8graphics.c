#include "chip8graphics.h"

void chip8graphics_init(C8graphics *graphics) {
        /* initializes struct: creates window, surface,
        initializes squares to be all black */

        graphics->window = NULL;
        graphics->screen_surface = NULL;
        graphics->square_side = 10;

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Can't initialize SDL. SDL_Error: %s\n",
                        SDL_GetError());
		exit(1);
	}

        int screen_width = 64 * graphics->square_side;
        int screen_height = 32 * graphics->square_side;

	graphics->window = SDL_CreateWindow("CHIP8 emulator",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                screen_width,
                screen_height,
                0);
	if (!graphics->window) {
		fprintf(stderr, "Can't create window. SDL_Error: %s\n",
                        SDL_GetError());
		exit(1);
	}

	int x;
	int y;
	for (y = 0; y < 32; y++)
		for (x = 0; x < 64; x++) {
			graphics->squares[y * 64 + x].x = x * graphics->square_side;
			graphics->squares[y * 64 + x].y = y * graphics->square_side;
			graphics->squares[y * 64 + x].w = graphics->square_side;
			graphics->squares[y * 64 + x].h = graphics->square_side;
		}

	graphics->screen_surface = SDL_GetWindowSurface(graphics->window);

	for (y = 0; y < 32; y++)
		for (x = 0; x < 64; x++)
			SDL_FillRect(graphics->screen_surface,
                                 &(graphics->squares[y * 64 + x]),
                                 SDL_MapRGB((graphics->screen_surface)->format, 0, 0, 0));
}

void chip8graphics_destroy(C8graphics *graphics) {
        SDL_DestroyWindow(graphics->window);
	SDL_Quit();
}

void chip8graphics_draw(C8graphics *graphics, C8 *emu) {
        /* draws pixel values from emu->graphics */
        int x;
        int y;
        Uint32 black = SDL_MapRGB((graphics->screen_surface)->format, 0, 0, 0);
        Uint32 white = SDL_MapRGB((graphics->screen_surface)->format, 0xFF, 0xFF, 0xFF);
        Uint32 color_to_fill;
        for (y = 0; y < 32; y++)
		for (x = 0; x < 64; x++) {
                        color_to_fill = emu->graphics[y * 64 + x] == 0 ? black : white;
			SDL_FillRect(graphics->screen_surface,
                                 &(graphics->squares[y * 64 + x]),
                                 color_to_fill);
                         }
}
