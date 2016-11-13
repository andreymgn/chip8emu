#ifndef __CHIP8_H__
#define __CHIP8_H__

#include <stdbool.h>
#include <stdio.h>
#include <SDL.h>

#define MEMORY_SIZE 4096

extern const unsigned char chip8_fontset[80];

typedef struct chip8 {
        FILE *game;

        unsigned short opcode;
        unsigned char memory[MEMORY_SIZE];
        unsigned char V[16];
        unsigned short I;
        unsigned short pc;
        unsigned char graphics[64 * 32];
        unsigned char delay_timer;
        unsigned char sound_timer;
        unsigned short stack[16];
        unsigned short sp;
        unsigned char key[16];
        bool should_draw;
        bool should_quit;
} C8;

void chip8_start(char*);
void chip8_load_game(C8*, char*);
void chip8_init(C8*);
void chip8_execute_opcode(C8*);
void chip8_handle_events(C8*, SDL_Event*);

#endif /* __CHIP8_H__ */
