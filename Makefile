# ------------------------------------------------
# Generic Makefile
#
# Author: yanick.rochon@gmail.com
# Date  : 2011-08-10
#
# Changelog :
#   2010-11-05 - first version
#   2011-08-10 - added structure : sources, objects, binaries
#                thanks to http://stackoverflow.com/users/128940/beta
# ------------------------------------------------

# project name (generate executable with this name)
MKDIR_P  = mkdir -p
TARGET   = chip8emu

CC       = gcc
# compiling flags here
SDL_CFLAGS := $(shell sdl2-config --cflags)
WARNINGS = -pedantic -Wall -Wshadow -Wpointer-arith -Wcast-qual \
	-Wstrict-prototypes -Wmissing-prototypes
CFLAGS   = -std=c99 $(WARNINGS) -I. $(SDL_CFLAGS) -O2

LINKER   = gcc -o
# linking flags here
SDL_LDFLAGS := $(shell sdl2-config --libs)
LFLAGS   = -I. $(SDL_LDFLAGS)

# change these to proper directories where each file should be
SRCDIR   = .
OBJDIR   = obj
BINDIR   = bin

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
rm       = rm -f


$(BINDIR)/$(TARGET): $(OBJECTS)
	$(MKDIR_P) $(BINDIR)
	@$(LINKER) $@ $(OBJECTS) $(LFLAGS)
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(MKDIR_P) $(OBJDIR)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	@$(rm) $(OBJECTS)
	@echo "Cleanup complete!"

.PHONY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
