# Simple Makefile for stb_font_c

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -I.

# Try SDL2 first (default)
SDL_CFLAGS = $(shell sdl2-config --cflags 2>/dev/null || echo "")
SDL_LIBS = $(shell sdl2-config --libs 2>/dev/null || echo "")

# If SDL2 is not available, try SDL3
ifeq ($(SDL_LIBS),)
    SDL_CFLAGS = $(shell sdl3-config --cflags 2>/dev/null || echo "")
    SDL_LIBS = $(shell sdl3-config --libs 2>/dev/null || echo "-lSDL3")
endif

# If still not found, try pkg-config
ifeq ($(SDL_LIBS),)
    SDL_CFLAGS = $(shell pkg-config --cflags sdl2 2>/dev/null || pkg-config --cflags sdl3 2>/dev/null || echo "")
    SDL_LIBS = $(shell pkg-config --libs sdl2 2>/dev/null || pkg-config --libs sdl3 2>/dev/null || echo "")
endif

# Add SDL and generic texture renderer support flags
CFLAGS += -DSTB_FONT_SDL_ENABLED -DSTB_FONT_TEXTURE_RENDERER_ENABLED

# Check if SDL is available
ifeq ($(SDL_LIBS),)
    $(error SDL not found. Please install SDL2 or SDL3 development libraries)
endif

# Target files
TARGETS = example simple_example multi_language_example
OBJS = stb_font_cache.o stb_font_simple.o sdl_render.o example.o simple_example.o multi_language_example.o

# Default target
all: $(TARGETS)

# Compile library
stb_font_cache.o: stb_font_cache.c stb_font_cache.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Compile simple wrapper
stb_font_simple.o: stb_font_simple.c stb_font_simple.h stb_font_cache.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Compile example
example.o: example.c stb_font_cache.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Link example
example: example.o stb_font_cache.o sdl_render.o
	$(CC) $(CFLAGS) $^ -o $@ $(SDL_LIBS) -lm

# Compile simple_example
simple_example.o: simple_example.c stb_font_simple.h stb_font_cache.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Link simple_example
simple_example: simple_example.o stb_font_cache.o stb_font_simple.o sdl_render.o
	$(CC) $(CFLAGS) $^ -o $@ $(SDL_LIBS) -lm

# Compile multi_language_example
multi_language_example.o: multi_language_example.c stb_font_cache.h 
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Link multi_language_example
multi_language_example: multi_language_example.o stb_font_cache.o sdl_render.o
	$(CC) $(CFLAGS) $^ -o $@ $(SDL_LIBS) -lm

# Clean
clean:
	rm -f $(OBJS) $(TARGETS)

# Run example
run: example
	./example

# Run multi_language_example
run_multi: multi_language_example
	./multi_language_example

# Install (simple)
install: stb_font_cache.h stb_font_cache.c
	install -d $(DESTDIR)/usr/local/include
	install -m 644 $< $(DESTDIR)/usr/local/include/

.PHONY: all clean run install
