#include <SDL.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "chip8.h"
#include "chip8graphics.h"

void chip8_start(char *game_path) {
        C8 emu;
        C8graphics graphics;
        SDL_Event event;

        chip8_load_game(&emu, game_path);
        chip8_init(&emu);
        chip8_init_graphics(&graphics);

        while (!emu.should_quit) {
                if (SDL_PollEvent(&event))
                        continue;
                chip8_execute_opcode(&emu);

                if (emu.should_draw)
                        chip8graphics_draw(&graphics, &emu);
                chip8_handle_events(&emu, &event);
                SDL_Delay(15);
        }

        chip8graphics_destroy(&graphics);
}

void chip8_load_game(C8 *emu, char *game_path) {
        emu->game = fopen(game_path, "rb");
        if (!emu->game) {
                fprintf(stderr, "Error opening file %s\n", game_path);
                exit(1);
        }

        /* load game into memory */
        fread(emu->memory+0x200, 1, MEMORY_SIZE-0x200, emu->game);
}

void chip8_init(C8 *emu) {
        /* load fontset into memory */
        int i;
        for (i = 0; i < 80; i++)
                emu->memory[i] = chip8_fontset[i];

        /* clear registers */
        memset(emu->V, 0, sizeof(emu->V));

        /* clear graphics */
        memset(emu->graphics, 0, sizeof(emu->graphics));

        /* clear stack */
        memset(emu->stack, 0, sizeof(emu->stack));

        /* initialize program couner */
        emu->pc = 0x200;
        /* reset stack pointer */
        emu->sp = 0;
        /* reset current opcode */
        emu->opcode = 0;
        /* reset index register */
        emu->I = 0;

        emu->delay_timer = 0;
        emu->sound_timer = 0;

        emu->should_draw = true;
        emu->should_quit = false;
        srand(time(NULL));
}

void chip8_execute_opcode(C8 *emu) {
        unsigned short x;
        unsigned short y;
        unsigned short n;
        emu->opcode = emu->memory[emu->pc] << 8 | emu->memory[emu->pc + 1];

        switch (emu->opcode & 0xF000) {
        case 0x0000:
                switch (emu->opcode & 0x000F) {
                case 0x0000:
                /* 0x00E0 - clear screen */
                        memset(emu->graphics, 0, sizeof(emu->graphics));
                        emu->pc += 2;
                        break;

                case 0x000E:
                /* 0x00EE - return from subroutine */
                        emu->sp--;
                        emu->pc = emu->stack[emu->sp];
                        emu->pc += 2;
                        break;

                default:
                        fprintf(stderr, "Unknown opcode 0x%X\n", emu->opcode);
                        exit(1);
                }
                break;

        case 0x1000:
        /* 0x1NNN - jump to address NNN */
                emu->pc = emu->opcode & 0x0FFF;
                break;

        case 0x2000:
        /* 0x2NNN - calls subroutine at NNN */
                emu->stack[emu->sp] = emu->pc;
                emu->sp++;
                emu->pc = emu->opcode & 0x0FFF;
                break;

        case 0x3000:
        /* 0x3XNN - skips the next inxtruction if VX==NN */
                x = (emu->opcode & 0x0F00) >> 8;
                n = emu->opcode & 0x00FF;
                if (emu->V[x] == n)
                        emu->pc += 4;
                else
                        emu->pc += 2;
                break;

        case 0x4000:
        /* 0x4XNN - skips the next instruction if VX!=NN */
                x = (emu->opcode & 0x0F00) >> 8;
                n = emu->opcode & 0x00FF;
                if (emu->V[x] != n)
                        emu->pc += 4;
                else
                        emu->pc += 2;
                break;

        case 0x5000:
        /* 0x5XY0 - skips the next instruction if VX=VY */
                x = (emu->opcode & 0x0F00) >> 8;
                y = (emu->opcode & 0x00F0) >> 4;
                if (emu->V[x] == emu->V[y])
                        emu->pc += 4;
                else
                        emu->pc += 2;
                break;

        case 0x6000:
        /* 0x6XNN - sets VX to NN */
                x = (emu->opcode & 0x0F00) >> 8;
                n = emu->opcode & 0x00FF;
                emu->V[x] = n;
                emu->pc += 2;
                break;

        case 0x7000:
        /* 0x7XNN - adds NN to VX */
                x = (emu->opcode & 0x0F00) >> 8;
                n = emu->opcode & 0x00FF;
                emu->V[x] += n;
                emu->pc += 2;
                break;

        case 0x8000:
                switch (emu->opcode & 0x000F) {
                case 0x0000:
                /* 0x8XY0 - sets VX to the value of VY */
                        x = (emu->opcode & 0x0F00) >> 8;
                        y = (emu->opcode & 0x00F0) >> 4;
                        emu->V[x] = emu->V[y];
                        emu->pc += 2;
                        break;

                case 0x0001:
                /* 0x8XY1 - sets VX to VX | VY */
                        x = (emu->opcode & 0x0F00) >> 8;
                        y = (emu->opcode & 0x00F0) >> 4;
                        emu->V[x] |= emu->V[y];
                        emu->pc += 2;
                        break;

                case 0x0002:
                /* 0x8XY2 - sets VX to VX & VY */
                        x = (emu->opcode & 0x0F00) >> 8;
                        y = (emu->opcode & 0x00F0) >> 4;
                        emu->V[x] &= emu->V[y];
                        emu->pc += 2;
                        break;

                case 0x0003:
                /* 0x8XY3 - sets VX to VX ^ VY */
                        x = (emu->opcode & 0x0F00) >> 8;
                        y = (emu->opcode & 0x00F0) >> 4;
                        emu->V[x] ^= emu->V[y];
                        emu->pc += 2;
                        break;

                case 0x0004:
                /* 0x8XY4 - adds VY to VX.
                VF is set to 1 when there's a carry,
                and to 0 when there isn't */
                        x = (emu->opcode & 0x0F00) >> 8;
                        y = (emu->opcode & 0x00F0) >> 4;
                        if (emu->V[y] > (0xFF - emu->V[x]))
                                emu->V[0xF] = 1;
                        else
                                emu->V[0xF] = 0;
                        emu->V[x] += emu->V[y];
                        emu->pc += 2;
                        break;

                case 0x0005:
                /* 0x8XY5 - VY is subtracted from VX.
                VF is set to 0 when there's a borrow, and 1 when there isn't */
                        x = (emu->opcode & 0x0F00) >> 8;
                        y = (emu->opcode & 0x00F0) >> 4;
                        if (emu->V[y] > emu->V[x])
                                emu->V[0xF] = 0;
                        else
                                emu->V[0xF] = 1;
                        emu->V[x] -= emu->V[y];
                        emu->pc += 2;
                        break;

                case 0x0006:
                /* 0x8XY6 - shifts VX right by one.
                VF is set to the value of the least significant bit of VX
                before the shift */
                        x = (emu->opcode & 0x0F00) >> 8;
                        emu->V[0xF] = emu->V[x] & 0x1;
                        emu->V[x] >>= 1;
                        emu->pc += 2;
                        break;

                case 0x0007:
                /* 0x8XY7 - sets VX to VY - VX.
                VF is set to 0 when there's a borrow, and 1 when there isn't */
                        x = (emu->opcode & 0x0F00) >> 8;
                        y = (emu->opcode & 0x00F0) >> 4;
                        if (emu->V[y] > emu->V[x])
                                emu->V[0xF] = 0;
                        else
                                emu->V[0xF] = 1;
                        emu->V[x] = emu->V[y] - emu->V[x];
                        emu->pc += 2;
                        break;

                case 0x000E:
                /* 0x8XYE - Shifts VX left by one.
                VF is set to the value of the most significant bit of VX
                before the shift. */
                        x = (emu->opcode & 0x0F00) >> 8;
                        emu->V[0xF] = emu->V[x] >> 7;
                        emu->V[x] <<= 1;
                        emu->pc += 2;
                        break;

                default:
                        fprintf(stderr, "Unknown opcode 0x%X\n", emu->opcode);
                        exit(1);
                }
                break;

        case 0x9000:
        /* 0x9XY0 - skips the next instruction if VX != VY */
                x = (emu->opcode & 0x0F00) >> 8;
                y = (emu->opcode & 0x00F0) >> 4;
                if (emu->V[x] != emu->V[y])
                        emu->pc += 4;
                else
                        emu->pc += 2;
                break;

        case 0xA000:
        /* 0xANNN - sets I to the address NNN */
                emu->I = emu->opcode & 0xFFF;
                emu->pc += 2;
                break;

        case 0xB000: /* 0xBNNN - jumps to the address NNN + V0 */
                emu->pc = emu->opcode & 0xFFF + emu->V[0];
                break;

        case 0xC000:
        /* 0xCXNN - sets VX to the result of
        a bitwise and operation on a random number and NN */
                x = (emu->opcode & 0x0F00) >> 8;
                n = emu->opcode & 0x00FF;
                emu->V[x] = (rand() % 0xFF) & n;
                emu->pc += 2;
                break;

        case 0xD000: {
        /* Draws a sprite at coordinate (VX, VY)
        that has a width of 8 pixels and a height of N pixels.
        Each row of 8 pixels is read as bit-coded starting
        from memory location I;
        I value doesn’t change after the execution of this instruction.
        As described above, VF is set to 1 if any screen pixels are flipped
        from set to unset when the sprite is drawn,
        and to 0 if that doesn’t happen */
                x = (emu->opcode & 0x0F00) >> 8;
                y = (emu->opcode & 0x00F0) >> 4;
                n = emu->opcode & 0x000F;
                unsigned short pixel;
                emu->V[0xF] = 0;
                int xline;
                int yline;
                for (yline = 0; yline < n; yline++) {
                        pixel = emu->memory[emu->I + yline];
                        for (xline = 0; xline < 8; xline++) {
                                if ((pixel & (0x80 >> xline)) != 0) {
                                        int pixel_index = x+xline+(y+yline)*64;
                                        if (emu->graphics[pixel_index] == 1)
                                                emu->V[0xF] = 1;
                                        emu->graphics[pixel_index] ^= 1;
                                }
                        }
                }

                emu->should_draw = true;
                emu->pc += 2;
        }
        break;

        case 0xE000:
                switch (emu->opcode & 0x00FF) {
                case 0x009E:
                /* 0xEX9E - skips the next instruction
                if the key stored in VX is pressed */
                        x = (emu->opcode & 0x0F00) >> 8;
                        if (emu->key[emu->V[x]] != 0)
                                emu->pc += 4;
                        else
                                emu->pc += 2;
                        break;

                case 0x00A1:
                /* 0xEXA1 - skips the next instruction
                if the key stored in VX isn't pressed */
                        x = (emu->opcode & 0x0F00) >> 8;
                        if (emu->key[emu->V[x]] == 0)
                                emu->pc += 4;
                        else
                                emu->pc += 2;
                        break;

                default:
                        fprintf(stderr, "Unknown opcode 0x%X\n", emu->opcode);
                        exit(1);
        }
        break;

        case 0xF000:
                switch (emu->opcode & 0x00FF) {
                case 0x0007:
                /* 0xFX07 - sets VX to the value of the delay timer */
                        x = (emu->opcode & 0x0F00) >> 8;
                        emu->V[x] = emu->delay_timer;
                        emu->pc += 2;
                        break;

                case 0x000A: {
                /* 0xFX0A - a key press is awaited, and then stored in VX */
                        x = (emu->opcode & 0x0F00) >> 8;
                        bool key_pressed = false;

                        int i;
                        for (i = 0; i < 16; i++) {
                                if (emu->key[i] != 0) {
                                        emu->V[x] = i;
                                        key_pressed = true;
                                        break;
                                }
                        }
                        if (!key_pressed)
                                return;

                        emu->pc += 2;
                }
                break;

                case 0x0015:
                /* 0xFX15 - sets the delay timer to VX. */
                        x = (emu->opcode & 0x0F00) >> 8;
                        emu->delay_timer = emu->V[x];
                        emu->pc += 2;
                        break;

                case 0x0018:
                /* 0xFX18 - sets the sound timer to VX */
                        x = (emu->opcode & 0x0F00) >> 8;
                        emu->sound_timer = emu->V[x];
                        emu->pc += 2;
                        break;

                case 0x001E:
                /* 0xFX1E - adds VX to I.
                VF is set to 1 when range overflow (I+VX>0xFFF),
                and 0 when there isn't. */
                        x = (emu->opcode & 0x0F00) >> 8;
                        if (emu->I+emu->V[x] > 0xFFF)
                                emu->V[0xF] = 1;
                        else
                                emu->V[0xF] = 0;
                        emu->I += emu->V[x];
                        emu->pc += 2;
                        break;

                case 0x0029:
                /* 0xFX29 - sets I to the location of the sprite
                for the character in VX.
                Characters 0-F (in hexadecimal) are represented by a 4x5 font */
                        x = (emu->opcode & 0x0F00) >> 8;
                        emu->I = emu->V[x] * 0x5;
                        emu->pc += 2;
                        break;

                case 0x0033:
                /* 0xFX33 - Stores the Binary-coded decimal representation
                of VX at the addresses I, I plus 1, and I plus 2 */
                        x = (emu->opcode & 0x0F00) >> 8;
                        emu->memory[emu->I] = emu->V[x] / 100;
			emu->memory[emu->I + 1] = (emu->V[x] / 10) % 10;
			emu->memory[emu->I + 2] = (emu->V[x] % 100) % 10;
                        emu->pc += 2;
                        break;

                case 0x0055: {
                /* 0xFX55 - stores V0 to VX (including VX)
                in memory starting at address I */
                        x = (emu->opcode & 0x0F00) >> 8;
                        int i;
                        for (i = 0; i <= x; i++)
                                emu->memory[emu->I + i] = emu->V[x];
                        emu->I += x + 1;
                        emu->pc += 2;
                }
                break;

                case 0x0065: {
                /* 0xFX65 - fills V0 to VX (including VX)
                with values from memory starting at address I */
                        x = (emu->opcode & 0x0F00) >> 8;
                        int i;
                        for (i = 0; i <= x; i++)
                                emu->V[i] = emu->memory[emu->I + i];

                        emu->I += x + 1;
                        emu->pc += 2;
                }
                break;

                }
        }

        /* update timers */
        if (emu->delay_timer > 0)
                emu->delay_timer--;
        if (emu->sound_timer > 0) {
                if (emu->sound_timer == 1)
                        ;/* play sound */
                emu->sound_timer--;
        }
}

void chip8_handle_events(C8 *emu, SDL_Event *event) {
        switch (event->type) {
        case SDL_QUIT:
                emu->should_quit = true;
                break;
        case SDL_KEYDOWN:
                switch (event->key.keysym.sym) {
                case SDLK_1:
                        emu->key[0x1] = 1;
                        break;
                case SDLK_2:
                        emu->key[0x2] = 1;
                        break;
                case SDLK_3:
                        emu->key[0x3] = 1;
                        break;
                case SDLK_4:
                        emu->key[0xC] = 1;
                        break;
                case SDLK_q:
                        emu->key[0x4] = 1;
                        break;
                case SDLK_w:
                        emu->key[0x5] = 1;
                        break;
                case SDLK_e:
                        emu->key[0x6] = 1;
                        break;
                case SDLK_r:
                        emu->key[0xD] = 1;
                        break;
                case SDLK_a:
                        emu->key[0x7] = 1;
                        break;
                case SDLK_s:
                        emu->key[0x8] = 1;
                        break;
                case SDLK_d:
                        emu->key[0x9] = 1;
                        break;
                case SDLK_f:
                        emu->key[0xE] = 1;
                        break;
                case SDLK_z:
                        emu->key[0xA] = 1;
                        break;
                case SDLK_x:
                        emu->key[0x0] = 1;
                        break;
                case SDLK_c:
                        emu->key[0xB] = 1;
                        break;
                case SDLK_v:
                        emu->key[0xF] = 1;
                        break;
                }
                break;
        case SDL_KEYUP:
                switch (event->key.keysym.sym) {
                case SDLK_1:
                        emu->key[0x1] = 0;
                        break;
                case SDLK_2:
                        emu->key[0x2] = 0;
                        break;
                case SDLK_3:
                        emu->key[0x3] = 0;
                        break;
                case SDLK_4:
                        emu->key[0xC] = 0;
                        break;
                case SDLK_q:
                        emu->key[0x4] = 0;
                        break;
                case SDLK_w:
                        emu->key[0x5] = 0;
                        break;
                case SDLK_e:
                        emu->key[0x6] = 0;
                        break;
                case SDLK_r:
                        emu->key[0xD] = 0;
                        break;
                case SDLK_a:
                        emu->key[0x7] = 0;
                        break;
                case SDLK_s:
                        emu->key[0x8] = 0;
                        break;
                case SDLK_d:
                        emu->key[0x9] = 0;
                        break;
                case SDLK_f:
                        emu->key[0xE] = 0;
                        break;
                case SDLK_z:
                        emu->key[0xA] = 0;
                        break;
                case SDLK_x:
                        emu->key[0x0] = 0;
                        break;
                case SDLK_c:
                        emu->key[0xB] = 0;
                        break;
                case SDLK_v:
                        emu->key[0xF] = 0;
                        break;
                }
                break;
        }
}
