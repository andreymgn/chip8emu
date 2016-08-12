#include <stdio.h>

#include "chip8.h"

int main(int argc, char *argv[]) {
        if (argc < 2) {
                printf("Usage: chip8emu <game_to_run>\n");
                return 0;
        }
        chip8_start(argv[1]);

        return 0;
}
