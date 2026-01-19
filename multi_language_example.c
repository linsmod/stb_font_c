/**
 * @file multi_language_example.c
 * @brief Example demonstrating multi-language text rendering with multiple fonts
 * 
 * This example shows how to:
 * - Load multiple fonts for different languages/scripts
 * - Add fallback fonts for characters not in the main font
 * - Render text in various languages (Latin, CJK, Arabic, Hebrew, etc.)
 * 
 * Public Domain / CC0
 */

#include "stb_font_cache.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Draw a background rectangle for text
 */
static void draw_text_background(SDL_Renderer* renderer, int x, int y, 
                                   stb_font_cache_t* cache, const char* text, 
                                   uint8_t bg_r, uint8_t bg_g, uint8_t bg_b) {
    int text_width, text_height;
    stb_font_get_text_size(cache, text, -1, &text_width, &text_height);
    
    SDL_SetRenderDrawColor(renderer, bg_r, bg_g, bg_b, 255);
    SDL_Rect bg = {x - 5, y - 5, text_width + 10, text_height + 10};
    SDL_RenderFillRect(renderer, &bg);
}

/**
 * @brief Draw a background rectangle for formatted text
 */
static void draw_formatted_text_background(SDL_Renderer* renderer, int x, int y, 
                                            stb_font_cache_t* cache, const char* text,
                                            const stb_font_text_format_t* format,
                                            uint8_t bg_r, uint8_t bg_g, uint8_t bg_b) {
    int text_width, text_height;
    stb_font_get_formatted_text_size(cache, text, format, -1, &text_width, &text_height);
    
    SDL_SetRenderDrawColor(renderer, bg_r, bg_g, bg_b, 255);
    SDL_Rect bg = {x - 5, y - 5, text_width + 10, text_height + 10};
    SDL_RenderFillRect(renderer, &bg);
}

/**
 * @brief Helper function to load font from file into memory
 */
static int load_font_from_file(const char* filename, stb_font_memory_t* mem) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open font file: %s\n", filename);
        return -1;
    }
    
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (stb_font_memory_alloc(mem, file_size) != 0) {
        fprintf(stderr, "Failed to allocate memory for font: %s\n", filename);
        fclose(fp);
        return -1;
    }
    
    size_t read = fread(mem->data, 1, file_size, fp);
    fclose(fp);
    
    if (read != (size_t)file_size) {
        fprintf(stderr, "Failed to read font file: %s\n", filename);
        stb_font_memory_free(mem);
        return -1;
    }
    
    printf("Loaded font: %s (%ld bytes)\n", filename, file_size);
    return 0;
}

extern texture_renderer_ops_t* stb_font_create_renderer_funcs();
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    printf("=== Multi-Language Font Rendering Example ===\n\n");
    
    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL\n");
        return 1;
    }
    
    /* Get DPI information for automatic scaling */
    float ddpi, hdpi, vdpi;
    float dpi_scale = 1.0f;
    
    if (SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi) == 0) {
        /* Calculate scale factor based on DPI (96 DPI is considered standard) */
        dpi_scale = hdpi / 96.0f;
        printf("Display DPI: %.1f (horizontal), %.1f (vertical), %.1f (diagonal)\n", hdpi, vdpi, ddpi);
        printf("DPI scale factor: %.2f\n", dpi_scale);
    } else {
        printf("Could not get DPI information, using default scale\n");
    }
    
    /* Create window with DPI-scaled dimensions */
    const int base_width = 1000;
    const int base_height = 800;
    const int width = (int)(base_width * dpi_scale);
    const int height = (int)(base_height * dpi_scale);
    
    printf("Window size: %dx%d (scaled from %dx%d)\n", width, height, base_width, base_height);
    
    SDL_Window* window = SDL_CreateWindow("Multi-Language Example",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        width, height,
                                        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
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
    /* Create font cache */
    stb_font_cache_t* cache = stb_font_cache_create(renderer_funcs, renderer);
    if (!cache) {
        fprintf(stderr, "Failed to create font cache\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    printf("Font cache created successfully\n");
    
    /* Set font size with DPI scaling */
    const int base_font_size = 24;
    const int font_size = (int)(base_font_size * dpi_scale);
    stb_font_cache_set_face_size(cache, font_size);
    printf("Font size set to %d pixels (scaled from %d)\n", font_size, base_font_size);
    
    /* Load main Latin font (with bold, italic, bold-italic variants) */
    printf("\n=== Loading Fonts ===\n");
    
    stb_font_memory_t noto_regular = {0};
    stb_font_memory_t noto_bold = {0};
    stb_font_memory_t noto_italic = {0};
    stb_font_memory_t noto_bold_italic = {0};
    
    if (load_font_from_file("fonts/NotoSans-Regular.ttf", &noto_regular) == 0) {
        if (stb_font_load_managed(cache, &noto_regular, 0) != 0) {
            fprintf(stderr, "Failed to load main font\n");
        }
    }
    
    if (load_font_from_file("fonts/NotoSans-Bold.ttf", &noto_bold) == 0) {
        stb_font_add_format_font_managed(cache, STB_FONT_FORMAT_BOLD, &noto_bold, 0);
    }
    
    if (load_font_from_file("fonts/NotoSans-Italic.ttf", &noto_italic) == 0) {
        stb_font_add_format_font_managed(cache, STB_FONT_FORMAT_ITALIC, &noto_italic, 0);
    }
    
    if (load_font_from_file("fonts/NotoSans-BoldItalic.ttf", &noto_bold_italic) == 0) {
        stb_font_add_format_font_managed(cache, 
                                         STB_FONT_FORMAT_BOLD | STB_FONT_FORMAT_ITALIC, 
                                         &noto_bold_italic, 0);
    }
    
    /* Load fallback fonts for different scripts */
    stb_font_memory_t noto_armenian = {0};
    if (load_font_from_file("fonts/NotoSansArmenian-Regular.ttf", &noto_armenian) == 0) {
        stb_font_add_managed(cache, &noto_armenian, 0);
    }
    
    stb_font_memory_t noto_georgian = {0};
    if (load_font_from_file("fonts/NotoSansGeorgian-Regular.ttf", &noto_georgian) == 0) {
        stb_font_add_managed(cache, &noto_georgian, 0);
    }
    
    stb_font_memory_t noto_hebrew = {0};
    if (load_font_from_file("fonts/NotoSansHebrew-Regular.ttf", &noto_hebrew) == 0) {
        stb_font_add_managed(cache, &noto_hebrew, 0);
    }
    
    stb_font_memory_t noto_devanagari = {0};
    if (load_font_from_file("fonts/NotoSansDevanagari-Regular.ttf", &noto_devanagari) == 0) {
        stb_font_add_managed(cache, &noto_devanagari, 0);
    }
    
    stb_font_memory_t noto_arabic = {0};
    if (load_font_from_file("fonts/NotoSansArabic-Regular.ttf", &noto_arabic) == 0) {
        stb_font_add_managed(cache, &noto_arabic, 0);
    }
    
    stb_font_memory_t noto_thai = {0};
    if (load_font_from_file("fonts/NotoSansThai-Regular.ttf", &noto_thai) == 0) {
        stb_font_add_managed(cache, &noto_thai, 0);
    }
    
    stb_font_memory_t noto_cjk = {0};
    if (load_font_from_file("fonts/NotoSansCJKjp-Regular.otf", &noto_cjk) == 0) {
        stb_font_add_managed(cache, &noto_cjk, 0);
    }
    
    printf("\n=== Font Loading Complete ===\n");
    
    /* Get font metrics */
    printf("\nFont Metrics:\n");
    printf("  Ascent: %d\n", cache->ascent);
    printf("  Descent: %d\n", cache->descent);
    printf("  Line gap: %d\n", cache->line_gap);
    printf("  Baseline: %d\n", cache->baseline);
    printf("  Row size: %d\n", cache->row_size);
    printf("  Scale: %.3f\n", cache->scale);
    
    /* Sample text in multiple languages */
    printf("\n=== Starting Rendering ===\n");
    printf("Press ESC or close window to exit\n\n");
    
    int running = 1;
    int y = 20;
    const int left_x = (int)(20 * dpi_scale);
    const int right_x = (int)(520 * dpi_scale);
    const int line_height = (int)(cache->scale * cache->row_size) + (int)(8 * dpi_scale);
    
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
        
        /* Clear screen with dark background */
        SDL_SetRenderDrawColor(renderer, 25, 25, 35, 255);
        SDL_RenderClear(renderer);
        
        /* Reset y position and font size with DPI scaling */
        y = (int)(20 * dpi_scale);
        const int base_font_increment = 1;
        int font_size_current = (int)(24 * dpi_scale);
        stb_font_cache_set_face_size(cache, font_size_current);
        
        /* Title with light color */
        stb_font_text_format_t title_fmt = stb_font_format_color(255, 255, 255, 255);
        draw_formatted_text_background(renderer, left_x, y, cache, 
                                       "Multi-Language Font Rendering", 
                                       &title_fmt, 60, 100, 140);
        stb_font_draw_text_formatted(cache, left_x, y, "Multi-Language Font Rendering", &title_fmt, -1);
        draw_formatted_text_background(renderer, right_x, y, cache, 
                                       "多语言字体渲染", 
                                       &title_fmt, 60, 100, 140);
        stb_font_draw_text_formatted(cache, right_x, y, "多语言字体渲染", &title_fmt, -1);
        y += line_height + 10;
        
        /* Separator line */
        SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
        SDL_Rect sep = {(int)(10 * dpi_scale), y, width - (int)(20 * dpi_scale), 2};
        SDL_RenderFillRect(renderer, &sep);
        y += (int)(15 * dpi_scale);

        
        
        /* Latin scripts with light colors */
        stb_font_text_format_t label_fmt = stb_font_format_color(200, 200, 220, 255);
        stb_font_draw_text_formatted(cache, left_x, y, "Latin Scripts:", &label_fmt, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "拉丁文字:", &label_fmt, -1);
        y += line_height;
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        stb_font_text_format_t white_text = stb_font_format_color(255, 255, 255, 255);
        stb_font_draw_text_formatted(cache, left_x, y, "Euro Symbol: €.", &white_text, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "欢迎！Welcome！Bienvenue！", &white_text, -1);
        y += line_height;
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        stb_font_draw_text_formatted(cache, left_x, y, "Polish: Mogę jeść szkło.", &white_text, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "Romanian: Pot să mănânc sticlă.", &white_text, -1);
        y += line_height;
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        stb_font_draw_text_formatted(cache, left_x, y, "Íslenska: Ég get etið gler.", &white_text, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "Greek: Μπορώ να φάω σπασμένα γυαλιά.", &white_text, -1);
        y += line_height + (int)(10 * dpi_scale);
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        /* CJK scripts with light colors */
        stb_font_draw_text_formatted(cache, left_x, y, "CJK Scripts:", &label_fmt, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "中日韩文字:", &label_fmt, -1);
        y += line_height;
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        stb_font_draw_text_formatted(cache, left_x, y, "Chinese: 我能吞下玻璃而不伤身体。", &white_text, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "Traditional: 我能吞下玻璃而不傷身體。", &white_text, -1);
        y += line_height;
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        stb_font_draw_text_formatted(cache, left_x, y, "Japanese: 私はガラスを食べられます。", &white_text, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "Korean: 저는 유리를 먹을 수 있어요.", &white_text, -1);
        y += line_height + (int)(10 * dpi_scale);
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        /* Other scripts with light colors */
        stb_font_draw_text_formatted(cache, left_x, y, "Other Scripts:", &label_fmt, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "其他文字:", &label_fmt, -1);
        y += line_height;
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        stb_font_draw_text_formatted(cache, left_x, y, "Armenian: Կրնամ ապակի ուտել.", &white_text, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "Georgian: მინას ვჭამ და არა მტკივა.", &white_text, -1);
        y += line_height;
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        stb_font_draw_text_formatted(cache, left_x, y, "Hebrew: אני יכול לאכול זכוכית.", &white_text, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "Arabic: أنا قادر على أكل الزجاج.", &white_text, -1);
        y += line_height;
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        stb_font_draw_text_formatted(cache, left_x, y, "Hindi: मैं काँच खा सकता हूँ.", &white_text, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "Thai: ฉันกินกระจกได้.", &white_text, -1);
        y += line_height + (int)(10 * dpi_scale);
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        /* Formatted text examples with light colors */
        stb_font_draw_text_formatted(cache, left_x, y, "Formatted Text:", &label_fmt, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "格式化文本:", &label_fmt, -1);
        y += line_height;
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        stb_font_text_format_t bold = stb_font_format_color(255, 255, 255, 255);
        bold.format = STB_FONT_FORMAT_BOLD;
        stb_font_draw_text_formatted(cache, left_x, y, "Bold text in multiple languages", &bold, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "多语言粗体文本", &bold, -1);
        y += line_height;
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        stb_font_text_format_t underline = stb_font_format_color(255, 255, 200, 255);
        underline.format = STB_FONT_FORMAT_UNDERLINE;
        stb_font_draw_text_formatted(cache, left_x, y, "Underlined: English 日本語 中文", &underline, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "下划线: Hello 世界 こんにちは", &underline, -1);
        y += line_height;
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        stb_font_text_format_t red = stb_font_format_color(255, 150, 150, 255);
        red.format = STB_FONT_FORMAT_BOLD;
        stb_font_draw_text_formatted(cache, left_x, y, "Red bold: العربية עברית 中文", &red, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "红色粗体: 你好 こんにちは مرحبا", &red, -1);
        y += line_height;
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        stb_font_text_format_t blue = stb_font_format_color(150, 150, 255, 255);
        blue.format = STB_FONT_FORMAT_ITALIC | STB_FONT_FORMAT_UNDERLINE;
        stb_font_draw_text_formatted(cache, left_x, y, "Blue italic+underline: Mix!", &blue, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "蓝色斜体+下划线: 混合！", &blue, -1);
        y += line_height + (int)(10 * dpi_scale);
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        /* CJK variants with light colors */
        stb_font_draw_text_formatted(cache, left_x, y, "CJK Variants:", &label_fmt, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "中日韩变体:", &label_fmt, -1);
        y += line_height;
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        stb_font_draw_text_formatted(cache, left_x, y, "CJK Variants: 判 与 海 直 約 返 次 今", &white_text, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "简繁体混排: 判 与 海 直 約 返 次 今", &white_text, -1);
        y += line_height + (int)(10 * dpi_scale);
        stb_font_cache_set_face_size(cache, font_size_current + (int)(base_font_increment * dpi_scale));
        font_size_current += (int)(base_font_increment * dpi_scale);
        
        /* Color gradient demo with brighter colors */
        stb_font_draw_text_formatted(cache, left_x, y, "Color Demo:", &label_fmt, -1);
        stb_font_draw_text_formatted(cache, right_x, y, "颜色演示:", &label_fmt, -1);
        y += line_height;
        
        for (int i = 0; i < 8; i++) {
            stb_font_text_format_t color_fmt = stb_font_format_color(
                (uint8_t)(150 + i * 12),
                (uint8_t)(180 + i * 8),
                (uint8_t)(230 - i * 15),
                255
            );
            char text[64];
            snprintf(text, sizeof(text), "Color %d: English 中文 %d", i + 1, i + 1);
            stb_font_draw_text_formatted(cache, left_x + (int)(50 * dpi_scale) + i * (int)(100 * dpi_scale), y, text, &color_fmt, -1);
            
            char text2[64];
            snprintf(text2, sizeof(text2), "颜色%d", i + 1);
            stb_font_draw_text_formatted(cache, right_x + (int)(50 * dpi_scale) + i * (int)(80 * dpi_scale), y, text2, &color_fmt, -1);
        }
        y += line_height + (int)(10 * dpi_scale);
        
        /* Footer with light color */
        stb_font_text_format_t footer_fmt = stb_font_format_color(200, 200, 220, 255);
        stb_font_draw_text_formatted(cache, left_x, height - (int)(30 * dpi_scale),
                                    "Press ESC to exit | Multiple fonts loaded for multi-language support",
                                    &footer_fmt, -1);
        
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
