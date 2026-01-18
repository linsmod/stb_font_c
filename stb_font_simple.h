/**
 * @file stb_font_simple.h
 * @brief Simplified wrapper for stb_font_cache
 * 
 * This wrapper provides a simpler, more ergonomic API for common use cases.
 * It handles memory management automatically and reduces boilerplate code.
 * 
 * Public Domain / CC0
 */

#ifndef STB_FONT_SIMPLE_H
#define STB_FONT_SIMPLE_H

#include "stb_font_cache.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*=============================================================================
 * Simplified Font Loading - Direct File Loading
 *===========================================================================*/

/**
 * @brief Load a font directly from a file (automatic memory management)
 * @param cache Font cache
 * @param filename Path to .ttf/.otf file
 * @param index Font index for .ttc files (usually 0)
 * @return 0 on success, non-zero on failure
 * 
 * This function handles file reading and memory allocation automatically.
 * Use this as the main font (sets up font metrics).
 */
int stb_font_load_file(stb_font_cache_t* cache, const char* filename, int index);

/**
 * @brief Add a fallback font directly from a file
 * @param cache Font cache
 * @param filename Path to .ttf/.otf file
 * @param index Font index for .ttc files (usually 0)
 * @return 0 on success, non-zero on failure
 * 
 * This adds a fallback font for characters not in the main font.
 */
int stb_font_add_file(stb_font_cache_t* cache, const char* filename, int index);

/**
 * @brief Add a format variant (bold/italic) directly from a file
 * @param cache Font cache
 * @param format_mask Format flags (STB_FONT_FORMAT_BOLD, STB_FONT_FORMAT_ITALIC, etc.)
 * @param filename Path to .ttf/.otf file
 * @param index Font index for .ttc files (usually 0)
 * @return 0 on success, non-zero on failure
 * 
 * Example: stb_font_add_format_file(cache, STB_FONT_FORMAT_BOLD, "font-bold.ttf", 0);
 */
int stb_font_add_format_file(stb_font_cache_t* cache, 
                             uint8_t format_mask, 
                             const char* filename, 
                             int index);

/*=============================================================================
 * Batch Font Loading
 *===========================================================================*/

/**
 * @brief Font loading configuration
 */
typedef struct {
    const char* filename;      /* Font file path */
    uint8_t format_mask;       /* Format flags (0 for fallback fonts, or STB_FONT_FORMAT_BOLD/ITALIC) */
    int index;                 /* Font index for .ttc files */
} stb_font_config_t;

/**
 * @brief Load multiple fonts at once
 * @param cache Font cache
 * @param configs Array of font configurations
 * @param count Number of configurations
 * @return Number of fonts successfully loaded, or -1 on error
 * 
 * The first font in the array becomes the main font (sets metrics).
 * Subsequent fonts are added as fallbacks or format variants.
 * 
 * Example:
 *   stb_font_config_t fonts[] = {
 *       {"NotoSans-Regular.ttf", 0, 0},
 *       {"NotoSans-Bold.ttf", STB_FONT_FORMAT_BOLD, 0},
 *       {"NotoSansCJKjp-Regular.otf", 0, 0},
 *   };
 *   stb_font_load_multiple(cache, fonts, 3);
 */
int stb_font_load_multiple(stb_font_cache_t* cache, 
                          const stb_font_config_t* configs, 
                          int count);

/*=============================================================================
 * One-Step Initialization
 *===========================================================================*/

/**
 * @brief Create and initialize font cache with main font in one call
 * @param renderer_funcs Renderer operations
 * @param renderer_context Renderer context (e.g., SDL_Renderer*)
 * @param font_size Font size in pixels
 * @param main_font Path to main font file
 * @return Initialized font cache, or NULL on failure
 * 
 * This combines stb_font_cache_create(), stb_font_cache_set_face_size(),
 * and stb_font_load_file() into a single call.
 * 
 * Example:
 *   stb_font_cache_t* cache = stb_font_cache_init_simple(
 *       renderer_funcs, renderer, 24, "fonts/NotoSans-Regular.ttf");
 */
stb_font_cache_t* stb_font_cache_init_simple(const texture_renderer_ops_t* renderer_funcs,
                                              void* renderer_context,
                                              int font_size,
                                              const char* main_font);

/**
 * @brief Create and initialize font cache with multiple fonts
 * @param renderer_funcs Renderer operations
 * @param renderer_context Renderer context
 * @param font_size Font size in pixels
 * @param configs Array of font configurations
 * @param count Number of configurations
 * @return Initialized font cache, or NULL on failure
 * 
 * This combines cache creation, size setting, and batch loading.
 * 
 * Example:
 *   stb_font_config_t fonts[] = {
 *       {"NotoSans-Regular.ttf", 0, 0},
 *       {"NotoSansCJK.otf", 0, 0},
 *   };
 *   stb_font_cache_t* cache = stb_font_cache_init_multiple(
 *       renderer_funcs, renderer, 24, fonts, 2);
 */
stb_font_cache_t* stb_font_cache_init_multiple(const texture_renderer_ops_t* renderer_funcs,
                                                void* renderer_context,
                                                int font_size,
                                                const stb_font_config_t* configs,
                                                int count);

/*=============================================================================
 * Convenience Loading by Script/Language
 *===========================================================================*/

/**
 * @brief Font family configuration for a specific script
 */
typedef struct {
    const char* regular;        /* Regular font */
    const char* bold;           /* Bold font (can be NULL) */
    const char* italic;         /* Italic font (can be NULL) */
    const char* bold_italic;    /* Bold+Italic font (can be NULL) */
} stb_font_family_t;

/**
 * @brief Load a complete font family (regular, bold, italic, bold-italic)
 * @param cache Font cache
 * @param family Font family configuration
 * @return 0 on success, non-zero on failure
 * 
 * Example:
 *   stb_font_family_t latin = {
 *       "NotoSans-Regular.ttf",
 *       "NotoSans-Bold.ttf",
 *       "NotoSans-Italic.ttf",
 *       "NotoSans-BoldItalic.ttf"
 *   };
 *   stb_font_load_family(cache, &latin);
 */
int stb_font_load_family(stb_font_cache_t* cache, const stb_font_family_t* family);

/**
 * @brief Add a font family as fallback (for specific scripts)
 * @param cache Font cache
 * @param family Font family configuration
 * @return 0 on success, non-zero on failure
 */
int stb_font_add_family(stb_font_cache_t* cache, const stb_font_family_t* family);

/*=============================================================================
 * Utility Functions
 *===========================================================================*/

/**
 * @brief Check if a file exists and is readable
 * @param filename File path
 * @return 1 if exists and readable, 0 otherwise
 */
int stb_font_file_exists(const char* filename);

/**
 * @brief Set default font directory for relative paths
 * @param directory Directory path (e.g., "fonts/")
 * 
 * After calling this, you can use relative filenames like:
 *   stb_font_load_file(cache, "NotoSans-Regular.ttf", 0);
 * instead of:
 *   stb_font_load_file(cache, "fonts/NotoSans-Regular.ttf", 0);
 */
void stb_font_set_directory(const char* directory);

/**
 * @brief Get current default font directory
 * @return Directory path, or NULL if not set
 */
const char* stb_font_get_directory(void);

#ifdef __cplusplus
}
#endif

#endif /* STB_FONT_SIMPLE_H */
