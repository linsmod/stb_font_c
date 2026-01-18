/**
 * @file example.c
 * @brief Example demonstrating usage of stb_font_cache C API
 * 
 * Public Domain / CC0
 */

#include "stb_font_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* SDL support is enabled via Makefile/CMake */
#include <SDL2/SDL.h>

/* Helper function to read a file into memory */
static int read_file(const char* filename, stb_font_memory_t* mem) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("Failed to open file: %s\n", filename);
        return -1;
    }
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (stb_font_memory_alloc(mem, size) != 0) {
        fclose(fp);
        return -1;
    }
    
    size_t read = fread(mem->data, 1, size, fp);
    fclose(fp);
    
    if (read != (size_t)size) {
        printf("Failed to read complete file: %s\n", filename);
        stb_font_memory_free(mem);
        return -1;
    }
    
    printf("Loaded font file: %s (%zu bytes)\n", filename, size);
    return 0;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    /* Initialize SDL */
    if (SDL_Init(0) < 0) {
        printf("Failed to initialize SDL\n");
        return 1;
    }
    
    /* Create window */
    int window_width = 800;
    int window_height = 600;
    
    SDL_Window* window = SDL_CreateWindow("stb_font_cache C Example",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        window_width, window_height,
                                        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Failed to create SDL window\n");
        SDL_Quit();
        return 1;
    }
    
    /* Create renderer */
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Failed to create SDL renderer\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    printf("SDL Renderer created successfully\n");
    
    /* Create font cache */
    stb_font_cache_t* cache = stb_font_cache_create();
    if (!cache) {
        printf("Failed to create font cache\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    /* Set font size */
    stb_font_cache_set_face_size(cache, 24);
    
    /* Load font */
    stb_font_memory_t font_mem = {0};
    const char* font_path = "fonts/NotoSans-Regular.ttf";
    
    if (read_file(font_path, &font_mem) != 0) {
        printf("Note: Font file not found. Using system default behavior.\n");
        printf("Please place a font file at: %s\n", font_path);
        /* Continue anyway - text will just not render properly */
    } else {
        /* Load font into cache */
        if (stb_font_load_managed(cache, &font_mem, 0) != 0) {
            printf("Failed to load font\n");
        }
        
        /* The memory is now managed by the cache, no need to free manually */
    }
    
    /* Bind SDL renderer */
    stb_font_sdl_bind_renderer(cache, renderer);
    
    /* Test strings */
    const char* test_strings[] = {
        "Hello, World!",
        "The quick brown fox jumps over the lazy dog.",
        "1234567890!@#$%^&*()",
        "UTF-8 Support: 你好 Привет مرحبا",
        "Tab\tSeparated\tText",
        "Multiple\nLines\nOf\nText",
        NULL
    };
    
    /* Main loop */
    int running = 1;
    int current_test = 0;
    Uint32 last_time = SDL_GetTicks();
    Uint32 frame_count = 0;
    
    /* Prerendered text variables (must be declared outside loop) */
    static stb_prerendered_text_t prerendered = {0};
    static int prerendered_created = 0;
    
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = 0;
                    } else if (event.key.keysym.sym == SDLK_SPACE) {
                        current_test++;
                        if (test_strings[current_test] == NULL) {
                            current_test = 0;
                        }
                    }
                    break;
            }
        }
        
        /* Update window size */
        SDL_GetWindowSize(window, &window_width, &window_height);
        
        /* Clear screen */
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderClear(renderer);
        
        /* Draw background grid */
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        for (int x = 0; x < window_width; x += 50) {
            SDL_RenderDrawLine(renderer, x, 0, x, window_height);
        }
        for (int y = 0; y < window_height; y += 50) {
            SDL_RenderDrawLine(renderer, 0, y, window_width, y);
        }
        
        /* Draw test strings */
        int y = 20;
        
        /* Title */
        stb_font_text_format_t title_fmt = stb_font_format_color(255, 255, 100, 255);
        stb_font_draw_text_formatted(cache, 20, y, "stb_font_cache C Example", &title_fmt, -1);
        y += (int)(cache->scale * cache->row_size) + 10;
        
        /* FPS counter */
        frame_count++;
        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_time >= 1000) {
            Uint32 fps = frame_count;
            frame_count = 0;
            last_time = current_time;
            
            char fps_text[64];
            snprintf(fps_text, sizeof(fps_text), "FPS: %u | Press SPACE to change test", fps);
            stb_font_text_format_t fps_fmt = stb_font_format_color(100, 255, 100, 255);
            stb_font_draw_text_formatted(cache, 20, y, fps_text, &fps_fmt, -1);
        } else {
            char fps_text[64];
            snprintf(fps_text, sizeof(fps_text), "FPS: Calculating... | Press SPACE to change test");
            stb_font_draw_text_formatted(cache, 20, y, fps_text, NULL, -1);
        }
        y += (int)(cache->scale * cache->row_size) + 20;
        
        /* Draw current test string */
        if (test_strings[current_test]) {
            stb_font_draw_text(cache, 20, y, test_strings[current_test], -1);
        }
        
        /* Draw formatted text examples */
        y += (int)(cache->scale * cache->row_size) * 2 + 20;
        
        stb_font_text_format_t red_bold = stb_font_format_color(255, 100, 100, 255);
        red_bold.format = STB_FONT_FORMAT_BOLD;
        stb_font_draw_text_formatted(cache, 20, y, "Red Bold Text", &red_bold, -1);
        
        y += (int)(cache->scale * cache->row_size);
        
        stb_font_text_format_t green_underline = stb_font_format_color(100, 255, 100, 255);
        green_underline.format = STB_FONT_FORMAT_UNDERLINE;
        stb_font_draw_text_formatted(cache, 20, y, "Green Underline Text", &green_underline, -1);
        
        y += (int)(cache->scale * cache->row_size);
        
        stb_font_text_format_t blue_strikethrough = stb_font_format_color(100, 100, 255, 255);
        blue_strikethrough.format = STB_FONT_FORMAT_STRIKETHROUGH;
        stb_font_draw_text_formatted(cache, 20, y, "Blue Strikethrough Text", &blue_strikethrough, -1);
        
        y += (int)(cache->scale * cache->row_size);
        
        stb_font_text_format_t yellow_combined = stb_font_format_color(255, 255, 100, 255);
        yellow_combined.format = STB_FONT_FORMAT_BOLD | STB_FONT_FORMAT_UNDERLINE;
        stb_font_draw_text_formatted(cache, 20, y, "Yellow Bold + Underline", &yellow_combined, -1);
        
        /* Prerendered text example */
        y += (int)(cache->scale * cache->row_size) * 2 + 20;
        
        if (!prerendered_created) {
            stb_font_text_format_t fmt = stb_font_format_color(255, 200, 100, 255);
            stb_font_sdl_render_formatted_to_object(cache, &prerendered,
                                                  "This text is prerendered", &fmt, -1);
            prerendered_created = 1;
        }
        
        stb_font_draw_text(cache, 20, y, "Prerendered text below:", -1);
        y += (int)(cache->scale * cache->row_size);
        stb_font_sdl_draw_prerendered(&prerendered, 20, y);
        
        /* Present */
        SDL_RenderPresent(renderer);
        
        /* Cap FPS */
        SDL_Delay(16);
    }
    
    /* Cleanup */
    if (prerendered_created) {
        stb_font_sdl_free_prerendered(&prerendered);
    }
    
    stb_font_cache_destroy(cache);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    printf("Shutdown complete\n");
    return 0;
}
