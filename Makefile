CC=gcc
CFLAGS=`sdl2-config --cflags --libs` -std=c99
DEPS=chip8.h chip8graphics.h
OBJ=main.o chip8.o chip8graphics.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

chip8emu: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm *.o chip8emu
