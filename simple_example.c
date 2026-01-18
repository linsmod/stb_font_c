/**
 * @file simple_example.c
 * @brief Simple example demonstrating basic usage of stb_font_cache
 * 
 * This is a minimal example showing:
 * - Creating a font cache
 * - Loading a font
 * - Drawing text with different formats
 * 
 * Public Domain / CC0
 */

#include "stb_font_cache.h"

/* SDL support is enabled via Makefile/CMake */
#include <SDL2/SDL.h>

/* Simple font data - minimal ASCII subset (for demo purposes)
 * In real usage, load a .ttf file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const unsigned char demo_font_data[] = {
    /* This would normally contain actual TTF font data */
    /* For this example, we expect the user to provide a real font file */
};
extern texture_renderer_ops_t* stb_font_create_renderer_funcs(SDL_Renderer* sdl_renderer);
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    printf("=== stb_font_cache Simple Example ===\n\n");
    
    /* Initialize SDL */
    if (SDL_Init(0) < 0) {
        fprintf(stderr, "Failed to initialize SDL\n");
        return 1;
    }
    
    /* Create window */
    const int width = 640;
    const int height = 480;
    
    SDL_Window* window = SDL_CreateWindow("Simple Example",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        width, height,
                                        SDL_WINDOW_SHOWN);
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
    const texture_renderer_ops_t* renderer_funcs = stb_font_create_renderer_funcs(renderer);
    /* Create font cache */
    stb_font_cache_t* cache = stb_font_cache_create(renderer_funcs,renderer);
    if (!cache) {
        fprintf(stderr, "Failed to create font cache\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    printf("Font cache created successfully\n");
    
    /* Set font size */
    const int font_size = 32;
    stb_font_cache_set_face_size(cache, font_size);
    printf("Font size set to %d pixels\n", font_size);
    
    /* Load font from file */
    const char* font_file = "fonts/NotoSans-Regular.ttf";
    
    /* Try to load font file using proper memory management */
    FILE* fp = fopen(font_file, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        /* Use stb_font_memory_alloc for proper memory management */
        stb_font_memory_t font_mem = {0};
        if (stb_font_memory_alloc(&font_mem, file_size) == 0) {
            size_t read = fread(font_mem.data, 1, file_size, fp);
            fclose(fp);
            
            if (read == (size_t)file_size) {
                if (stb_font_load_managed(cache, &font_mem, 0) == 0) {
                    printf("Font loaded successfully from: %s\n", font_file);
                } else {
                    printf("Failed to load font (invalid format?)\n");
                    stb_font_memory_free(&font_mem);
                }
            } else {
                stb_font_memory_free(&font_mem);
                printf("Failed to read font file\n");
            }
        } else {
            fclose(fp);
            printf("Failed to allocate memory for font\n");
        }
    } else {
        printf("Note: Font file not found at: %s\n", font_file);
        printf("Text rendering may not work properly without a font.\n");
        printf("Please place a TrueType font file at that location.\n");
    }
    
    /* Get font metrics */
    printf("\nFont Metrics:\n");
    printf("  Ascent: %d\n", cache->ascent);
    printf("  Descent: %d\n", cache->descent);
    printf("  Line gap: %d\n", cache->line_gap);
    printf("  Baseline: %d\n", cache->baseline);
    printf("  Row size: %d\n", cache->row_size);
    printf("  Scale: %.3f\n", cache->scale);
    
    /* Text measurement example */
    const char* test_text = "Hello, World!";
    int text_width, text_height;
    stb_font_get_text_size(cache, test_text, -1, &text_width, &text_height);
    printf("\nText Size for \"%s\":\n", test_text);
    printf("  Width: %d pixels\n", text_width);
    printf("  Height: %d pixels\n", text_height);
    
    /* Main loop */
    printf("\nRunning... Press ESC or close window to exit\n\n");
    
    int running = 1;
    
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
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderClear(renderer);
        
        /* Reset y position at the start of each frame */
        int y = 20;
        int left_x = 20;
        int right_x = 350;
        
        /* Draw title */
        stb_font_text_format_t title_fmt = stb_font_format_color(255, 255, 200, 255);
        stb_font_draw_text_formatted(cache, left_x, y, "stb_font_cache Simple Example", &title_fmt, -1);
        y += (int)(cache->scale * cache->row_size) + 10;
        
        /* Draw plain text */
        stb_font_draw_text(cache, left_x, y, "Plain text in default color", -1);
        stb_font_draw_text(cache, right_x, y, "默认颜色的纯文本", -1);
        y += (int)(cache->scale * cache->row_size) + 5;
        
        /* Draw colored text */
        y += 10;
        stb_font_text_format_t red = stb_font_format_color(255, 100, 100, 255);
        stb_font_draw_text_formatted(cache, left_x, y, "Red text", &red, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "红色文本", &red, -1);
        y += (int)(cache->scale * cache->row_size);
        
        stb_font_text_format_t green = stb_font_format_color(100, 255, 100, 255);
        stb_font_draw_text_formatted(cache, left_x, y, "Green text", &green, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "绿色文本", &green, -1);
        y += (int)(cache->scale * cache->row_size);
        
        stb_font_text_format_t blue = stb_font_format_color(100, 100, 255, 255);
        stb_font_draw_text_formatted(cache, left_x, y, "Blue text", &blue, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "蓝色文本", &blue, -1);
        y += (int)(cache->scale * cache->row_size);
        
        /* Draw formatted text */
        y += 10;
        stb_font_text_format_t bold = stb_font_format_flags(STB_FONT_FORMAT_BOLD);
        stb_font_draw_text_formatted(cache, left_x, y, "Bold text", &bold, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "粗体文本", &bold, -1);
        y += (int)(cache->scale * cache->row_size);
        
        stb_font_text_format_t underline = stb_font_format_flags(STB_FONT_FORMAT_UNDERLINE);
        stb_font_draw_text_formatted(cache, left_x, y, "Underlined text", &underline, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "下划线文本", &underline, -1);
        y += (int)(cache->scale * cache->row_size);
        
        stb_font_text_format_t strikethrough = stb_font_format_flags(STB_FONT_FORMAT_STRIKETHROUGH);
        stb_font_draw_text_formatted(cache, left_x, y, "Strikethrough text", &strikethrough, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "删除线文本", &strikethrough, -1);
        y += (int)(cache->scale * cache->row_size);
        
        /* Combined formats */
        y += 10;
        stb_font_text_format_t combined = stb_font_format_color(255, 200, 100, 255);
        combined.format = STB_FONT_FORMAT_BOLD | STB_FONT_FORMAT_UNDERLINE;
        stb_font_draw_text_formatted(cache, left_x, y, "Bold + Underline + Yellow", &combined, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "粗体+下划线+黄色", &combined, -1);
        y += (int)(cache->scale * cache->row_size);
        
        /* Multi-line text */
        y += 10;
        stb_font_draw_text(cache, left_x, y, "Line 1\nLine 2\nLine 3", -1);
        stb_font_draw_text(cache, right_x, y, "第一行\n第二行\n第三行", -1);
        y += (int)(cache->scale * cache->row_size) * 3 + 10;
        
        /* Additional Chinese examples */
        stb_font_text_format_t chinese_title = stb_font_format_color(255, 200, 100, 255);
        stb_font_draw_text_formatted(cache, left_x, y, "More Examples:", &chinese_title, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "更多示例：", &chinese_title, -1);
        y += (int)(cache->scale * cache->row_size) + 5;
        
        stb_font_draw_text(cache, left_x, y, "Hello, World!", -1);
        stb_font_draw_text(cache, right_x, y, "你好，世界！", -1);
        y += (int)(cache->scale * cache->row_size) + 5;
        
        stb_font_draw_text(cache, left_x, y, "Welcome to stb_font_cache", -1);
        stb_font_draw_text(cache, right_x, y, "欢迎使用 stb_font_cache", -1);
        y += (int)(cache->scale * cache->row_size) + 5;
        
        stb_font_draw_text(cache, left_x, y, "UTF-8 Support", -1);
        stb_font_draw_text(cache, right_x, y, "支持 UTF-8 编码", -1);
        y += (int)(cache->scale * cache->row_size) + 5;
        
        stb_font_draw_text(cache, left_x, y, "Chinese: 汉字, 标点, 123", -1);
        stb_font_draw_text(cache, right_x, y, "中文字符：汉字、标点、数字", -1);
        
        /* Present */
        SDL_RenderPresent(renderer);
        
        /* Cap FPS */
        SDL_Delay(16);
    }
    
    /* Cleanup */
    printf("\nCleaning up...\n");
    stb_font_cache_destroy(cache);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    printf("Done!\n");
    return 0;
}
