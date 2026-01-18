/**
 * @file simple_example.c
 * @brief Example demonstrating simplified stb_font_cache wrapper
 * 
 * This example shows the difference between the original API and the simplified wrapper.
 * 
 * Public Domain / CC0
 */

#include "stb_font_simple.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern texture_renderer_ops_t* stb_font_create_renderer_funcs();

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    printf("=== Simplified Font API Example ===\n\n");
    
    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL\n");
        return 1;
    }
    
    /* Create window */
    const int width = 1000;
    const int height = 800;
    
    SDL_Window* window = SDL_CreateWindow("Simplified API Example",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        width, height,
                                        SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        SDL_Quit();
        return 1;
    }
    
    /* Create renderer */
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Failed to create renderer\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    const texture_renderer_ops_t* renderer_funcs = stb_font_create_renderer_funcs();
    
    printf("=== Method 1: One-Step Initialization ===\n");
    printf("Just 1 line of code to create and initialize font cache!\n\n");
    
    /* Method 1: Ultra-simple - one line initialization */
    stb_font_cache_t* cache1 = stb_font_cache_init_simple(
        renderer_funcs, renderer, 24, "fonts/NotoSans-Regular.ttf");
    
    if (cache1) {
        printf("✓ Cache initialized with main font\n");
        stb_font_draw_text_formatted(cache1, 50, 50, "Method 1: One-step init works!", 
                                     &(stb_font_text_format_t){255, 255, 255, 255, 0, 0}, -1);
        stb_font_cache_destroy(cache1);
    }
    
    printf("\n=== Method 2: Batch Loading ===\n");
    printf("Load all fonts in a single function call\n\n");
    
    /* Set default font directory so we don't need to repeat "fonts/" */
    stb_font_set_directory("fonts");
    
    /* Method 2: Batch loading */
    stb_font_config_t fonts[] = {
        {"NotoSans-Regular.ttf", 0, 0},                    /* Main font */
        {"NotoSans-Bold.ttf", STB_FONT_FORMAT_BOLD, 0},    /* Bold variant */
        {"NotoSans-Italic.ttf", STB_FONT_FORMAT_ITALIC, 0},/* Italic variant */
        {"NotoSansCJKjp-Regular.otf", 0, 0},              /* CJK fallback */
        {"NotoSansArabic-Regular.ttf", 0, 0},             /* Arabic fallback */
    };
    
    stb_font_cache_t* cache2 = stb_font_cache_init_multiple(
        renderer_funcs, renderer, 24, fonts, sizeof(fonts) / sizeof(fonts[0]));
    
    if (cache2) {
        printf("✓ Loaded %zu fonts in one call!\n", sizeof(fonts) / sizeof(fonts[0]));
    }
    
    printf("\n=== Method 3: Font Family Loading ===\n");
    printf("Load complete font families (regular, bold, italic, bold-italic)\n\n");
    
    /* Method 3: Font family */
    stb_font_family_t latin_family = {
        .regular = "NotoSans-Regular.ttf",
        .bold = "NotoSans-Bold.ttf",
        .italic = "NotoSans-Italic.ttf",
        .bold_italic = "NotoSans-BoldItalic.ttf"
    };
    
    stb_font_cache_t* cache3 = stb_font_cache_init_simple(
        renderer_funcs, renderer, 24, "NotoSans-Regular.ttf");
    
    if (cache3) {
        if (stb_font_load_family(cache3, &latin_family) == 0) {
            printf("✓ Latin font family loaded\n");
        }
        
        /* Add other script families as fallbacks */
        stb_font_family_t cjk_family = {
            .regular = "NotoSansCJKjp-Regular.otf",
            .bold = NULL,
            .italic = NULL,
            .bold_italic = NULL
        };
        
        if (stb_font_add_family(cache3, &cjk_family) == 0) {
            printf("✓ CJK fallback family added\n");
        }
    }
    
    printf("\n=== Comparison: Original vs Simplified ===\n");
    printf("\nOriginal API (from multi_language_example.c):\n");
    printf("  1. Create stb_font_memory_t structures\n");
    printf("  2. Write custom load_font_from_file() helper\n");
    printf("  3. Call stb_font_memory_alloc()\n");
    printf("  4. fopen(), fseek(), fread() for each font\n");
    printf("  5. Call stb_font_load_managed() or stb_font_add_managed()\n");
    printf("  Total: ~40 lines of code just for loading fonts!\n");
    
    printf("\nSimplified API (this example):\n");
    printf("  Option 1: stb_font_cache_init_simple() - ONE LINE!\n");
    printf("  Option 2: stb_font_cache_init_multiple() - with font array\n");
    printf("  Option 3: stb_font_load_family() - for font families\n");
    printf("  Total: 3-5 lines of code!\n");
    
    /* Now let's use cache2 for rendering demo */
    if (!cache2) {
        fprintf(stderr, "Failed to create cache2\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    /* Main rendering loop */
    printf("\n=== Starting Rendering Demo ===\n");
    printf("Press ESC to exit\n\n");
    
    int running = 1;
    int y = 20;
    const int line_height = (int)(cache2->scale * cache2->row_size) + 8;
    
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                }
            }
        }
        
        /* Clear screen */
        SDL_SetRenderDrawColor(renderer, 25, 25, 35, 255);
        SDL_RenderClear(renderer);
        
        y = 20;
        
        /* Title */
        stb_font_text_format_t title = stb_font_format_color(255, 255, 255, 255);
        stb_font_draw_text_formatted(cache2, 20, y, "Simplified API Demo", &title, -1);
        y += line_height + 10;
        
        /* Separator */
        SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
        SDL_Rect sep = {10, y, width - 20, 2};
        SDL_RenderFillRect(renderer, &sep);
        y += 15;
        
        /* Multilingual text */
        stb_font_text_format_t white = stb_font_format_color(255, 255, 255, 255);
        
        stb_font_draw_text_formatted(cache2, 20, y, "English: Hello, World!", &white, -1);
        y += line_height;
        
        stb_font_draw_text_formatted(cache2, 20, y, "中文: 你好，世界！", &white, -1);
        y += line_height;
        
        stb_font_draw_text_formatted(cache2, 20, y, "日本語: こんにちは、世界！", &white, -1);
        y += line_height;
        
        stb_font_draw_text_formatted(cache2, 20, y, "한국어: 안녕하세요, 세계!", &white, -1);
        y += line_height;
        
        stb_font_draw_text_formatted(cache2, 20, y, "العربية: مرحبا بالعالم!", &white, -1);
        y += line_height;
        
        /* Bold text */
        stb_font_text_format_t bold = stb_font_format_color(255, 200, 100, 255);
        bold.format = STB_FONT_FORMAT_BOLD;
        stb_font_draw_text_formatted(cache2, 20, y, "Bold text with multiple fonts: 中文 日本語 العربية", &bold, -1);
        y += line_height;
        
        /* Italic text */
        stb_font_text_format_t italic = stb_font_format_color(100, 255, 200, 255);
        italic.format = STB_FONT_FORMAT_ITALIC;
        stb_font_draw_text_formatted(cache2, 20, y, "Italic: This is italic text 你好 مرحبا", &italic, -1);
        y += line_height + 10;
        
        /* Code comparison display */
        stb_font_text_format_t code_title = stb_font_format_color(100, 200, 255, 255);
        stb_font_draw_text_formatted(cache2, 20, y, "Code Comparison:", &code_title, -1);
        y += line_height;
        
        stb_font_text_format_t gray = stb_font_format_color(180, 180, 180, 255);
        stb_font_draw_text_formatted(cache2, 20, y, "Old: load_font_from_file(..., &mem); stb_font_load_managed(..., &mem, 0);", &gray, -1);
        y += line_height;
        
        stb_font_text_format_t green = stb_font_format_color(100, 255, 100, 255);
        stb_font_draw_text_formatted(cache2, 20, y, "New: stb_font_load_file(cache, \"font.ttf\", 0);", &green, -1);
        y += line_height;
        
        stb_font_draw_text_formatted(cache2, 20, y, "Or: stb_font_cache_init_simple(funcs, ctx, 24, \"font.ttf\");", &green, -1);
        y += line_height + 10;
        
        /* Footer */
        stb_font_text_format_t footer = stb_font_format_color(150, 150, 170, 255);
        stb_font_draw_text_formatted(cache2, 20, height - 30, 
                                    "Simplified API: Less code, same power. Press ESC to exit.", 
                                    &footer, -1);
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    /* Cleanup */
    printf("\nCleaning up...\n");
    stb_font_cache_destroy(cache2);
    if (cache3) stb_font_cache_destroy(cache3);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    printf("Done!\n");
    return 0;
}
