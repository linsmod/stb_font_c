/**
 * @file stb_font_cache.h
 * @brief C API for stb_truetype font rendering library
 * 
 * A lightweight C wrapper around stb_truetype for font rendering.
 * This library provides functionality for:
 * - Loading TrueType fonts
 * - Caching glyphs
 * - Measuring text dimensions
 * - Drawing formatted text with colors, bold, italic, underline, strikethrough
 * - Prerendering text to textures (for SDL)
 * 
 * Public Domain / CC0 - Feel free to use however you want
 */

#ifndef STB_FONT_CACHE_H
#define STB_FONT_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* Forward declarations */
struct stb_font_cache;
struct stb_glyph;

/**
 * @brief Font format flags
 */
typedef enum {
    STB_FONT_FORMAT_NONE           = 0 << 0,
    STB_FONT_FORMAT_BOLD           = 1 << 0,
    STB_FONT_FORMAT_ITALIC         = 1 << 1,
    STB_FONT_FORMAT_UNDERLINE      = 1 << 2,
    STB_FONT_FORMAT_STRIKETHROUGH  = 1 << 3,
    STB_FONT_FORMAT_COLOR_SET      = 1 << 7
} stb_font_format_t;

/**
 * @brief Text formatting structure
 */
typedef struct {
    uint8_t r, g, b, a;      /* RGBA color values */
    uint8_t format;           /* Format flags */
    uint8_t flags;            /* Internal flags */
} stb_font_text_format_t;

/**
 * @brief Glyph information structure
 */
typedef struct {
    int16_t advance;          /* Advance width */
    int16_t left_side_bearing;
    uint16_t font_idx;
    uint16_t padding;
    int16_t width;           /* Glyph bitmap width */
    int16_t height;          /* Glyph bitmap height */
    int16_t x_offset;        /* X offset from cursor position */
    int16_t y_offset;        /* Y offset from cursor position */
    void* texture;           /* Platform-specific texture handle */
} stb_glyph_t;

/**
 * @brief Memory management structure for owned font data
 */
typedef struct {
    unsigned char* data;
    size_t size;
    int owned;
} stb_font_memory_t;

/**
 * @brief Font cache structure
 * 
 * This is the main context for all font operations.
 * Multiple caches can be created for different fonts or sizes.
 */
typedef struct stb_font_cache {
    /* Font metrics (read-only, calculated after loading) */
    int ascent;
    int descent;
    int line_gap;
    int baseline;
    int row_size;
    int tab_width;
    float scale;
    float underline_thickness;
    float strikethrough_thickness;
    float underline_position;
    float strikethrough_position;
    
    /* Configuration (set before loading) */
    int face_size;
    int tab_width_in_spaces;
    void* user_data;
    
    /* Internal data */
    void* internal;          /* Internal implementation data */
} stb_font_cache_t;

/**
 * @brief Prerendered text structure
 */
typedef struct {
    int width;
    int height;
    void* texture;
    void* renderer;
} stb_prerendered_text_t;

/*=============================================================================
 * Core Functions
 *===========================================================================*/

/**
 * @brief Create a new font cache
 * @return Pointer to the new font cache, or NULL on failure
 */
stb_font_cache_t* stb_font_cache_create(void);

/**
 * @brief Destroy a font cache and free all resources
 * @param cache Font cache to destroy
 */
void stb_font_cache_destroy(stb_font_cache_t* cache);

/**
 * @brief Set the font size (must be called before loading a font)
 * @param cache Font cache
 * @param face_size Font size in pixels
 */
void stb_font_cache_set_face_size(stb_font_cache_t* cache, int face_size);

/**
 * @brief Get the scaled row height
 * @param cache Font cache
 * @return Row height in pixels
 */
int stb_font_cache_get_row_height(stb_font_cache_t* cache);

/*=============================================================================
 * Font Loading
 *===========================================================================*/

/**
 * @brief Load a TrueType font from buffer
 * @param cache Font cache
 * @param ttf_buffer Pointer to font data
 * @param ttf_buffer_len Size of font data
 * @param index Font index (for .ttc files), typically 0
 * @return 0 on success, non-zero on failure
 */
int stb_font_load(stb_font_cache_t* cache, 
                  const unsigned char* ttf_buffer, 
                  size_t ttf_buffer_len, 
                  int index);

/**
 * @brief Load a font and take ownership of the memory
 * @param cache Font cache
 * @param memory Memory structure containing font data (will be freed when cache is destroyed)
 * @param index Font index (for .ttc files), typically 0
 * @return 0 on success, non-zero on failure
 */
int stb_font_load_managed(stb_font_cache_t* cache, 
                         stb_font_memory_t* memory, 
                         int index);

/**
 * @brief Add a fallback font
 * @param cache Font cache
 * @param ttf_buffer Pointer to font data
 * @param ttf_buffer_len Size of font data
 * @param index Font index (for .ttc files), typically 0
 * @return 0 on success, non-zero on failure
 */
int stb_font_add(stb_font_cache_t* cache, 
                 const unsigned char* ttf_buffer, 
                 size_t ttf_buffer_len, 
                 int index);

/**
 * @brief Add a fallback font and take ownership of the memory
 * @param cache Font cache
 * @param memory Memory structure containing font data (will be freed when cache is destroyed)
 * @param index Font index (for .ttc files), typically 0
 * @return 0 on success, non-zero on failure
 */
int stb_font_add_managed(stb_font_cache_t* cache, 
                        stb_font_memory_t* memory, 
                        int index);

/**
 * @brief Add a format variant (bold, italic, etc.) for the last loaded font
 * @param cache Font cache
 * @param format_mask Format flags (STB_FONT_FORMAT_*)
 * @param ttf_buffer Pointer to font data
 * @param ttf_buffer_len Size of font data
 * @param index Font index (for .ttc files), typically 0
 * @return 0 on success, non-zero on failure
 */
int stb_font_add_format_font(stb_font_cache_t* cache, 
                            uint8_t format_mask, 
                            const unsigned char* ttf_buffer, 
                            size_t ttf_buffer_len, 
                            int index);

/**
 * @brief Add a format variant and take ownership of the memory
 * @param cache Font cache
 * @param format_mask Format flags (STB_FONT_FORMAT_*)
 * @param memory Memory structure containing font data (will be freed when cache is destroyed)
 * @param index Font index (for .ttc files), typically 0
 * @return 0 on success, non-zero on failure
 */
int stb_font_add_format_font_managed(stb_font_cache_t* cache, 
                                    uint8_t format_mask, 
                                    stb_font_memory_t* memory, 
                                    int index);

/*=============================================================================
 * Text Measurement
 *===========================================================================*/

/**
 * @brief Measure the size of a plain text string
 * @param cache Font cache
 * @param text UTF-8 string to measure
 * @param max_len Maximum length in bytes (use -1 for null-terminated)
 * @param width_out Output width (can be NULL)
 * @param height_out Output height (can be NULL)
 */
void stb_font_get_text_size(stb_font_cache_t* cache, 
                           const char* text, 
                           int max_len, 
                           int* width_out, 
                           int* height_out);

/**
 * @brief Measure the size of formatted text
 * @param cache Font cache
 * @param text UTF-8 string to measure
 * @param format Text format
 * @param max_len Maximum length in bytes (use -1 for null-terminated)
 * @param width_out Output width (can be NULL)
 * @param height_out Output height (can be NULL)
 */
void stb_font_get_formatted_text_size(stb_font_cache_t* cache, 
                                     const char* text, 
                                     const stb_font_text_format_t* format, 
                                     int max_len, 
                                     int* width_out, 
                                     int* height_out);

/*=============================================================================
 * Rendering - Platform-Specific Functions
 *===========================================================================*/

/**
 * @brief Draw plain text at specified position
 * 
 * Note: This function must be implemented by a platform-specific frontend
 * (e.g., SDL, BGFX, etc.)
 * 
 * @param cache Font cache
 * @param x X position
 * @param y Y position
 * @param text UTF-8 string to draw
 * @param max_len Maximum length in bytes (use -1 for null-terminated)
 * @return X position after drawing
 */
int stb_font_draw_text(stb_font_cache_t* cache, 
                      int x, int y, 
                      const char* text, 
                      int max_len);

/**
 * @brief Draw formatted text at specified position
 * 
 * Note: This function must be implemented by a platform-specific frontend
 * 
 * @param cache Font cache
 * @param x X position
 * @param y Y position
 * @param text UTF-8 string to draw
 * @param format Text format
 * @param max_len Maximum length in bytes (use -1 for null-terminated)
 * @return X position after drawing
 */
int stb_font_draw_text_formatted(stb_font_cache_t* cache, 
                                int x, int y, 
                                const char* text, 
                                const stb_font_text_format_t* format, 
                                int max_len);

/*=============================================================================
 * Generic Texture-Based Renderer Functions
 *===========================================================================*/

#ifdef STB_FONT_TEXTURE_RENDERER_ENABLED

/* Forward declarations for the generic texture renderer interface */
typedef struct texture_renderer texture_renderer_t;

/* Generic texture operations that any renderer must implement */
typedef struct {
    /* Texture management */
    void* (*create_texture_from_surface)(texture_renderer_t* renderer, void* surface);
    void* (*create_texture)(texture_renderer_t* renderer, int width, int height, int format);
    void (*destroy_texture)(texture_renderer_t* renderer, void* texture);
    void (*set_texture_blend_mode)(texture_renderer_t* renderer, void* texture, int blend_mode);
    void (*set_texture_color_mod)(texture_renderer_t* renderer, void* texture, uint8_t r, uint8_t g, uint8_t b);
    
    /* Surface management (optional - can be NULL if not supported) */
    void* (*create_surface_from_rgba)(texture_renderer_t* renderer, unsigned char* rgba, int width, int height, int pitch);
    void (*free_surface)(texture_renderer_t* renderer, void* surface);
    
    /* Rendering operations */
    void (*render_copy)(texture_renderer_t* renderer, void* texture, const void* src_rect, const void* dst_rect);
    void (*set_render_target)(texture_renderer_t* renderer, void* target);
    void* (*get_render_target)(texture_renderer_t* renderer);
    void (*set_render_draw_color)(texture_renderer_t* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void (*set_render_draw_blend_mode)(texture_renderer_t* renderer, int blend_mode);
    void (*render_fill_rect)(texture_renderer_t* renderer, const void* rect);
    void (*render_clear)(texture_renderer_t* renderer);
    
    /* Utility */
    int (*get_error)(texture_renderer_t* renderer, char* buffer, int buffer_size);
} texture_renderer_ops_t;

/* Main texture renderer structure */
struct texture_renderer {
    texture_renderer_ops_t ops;
    void* user_data;  /* Renderer-specific data (e.g., SDL_Renderer) */
};

/* Generic rectangle structure */
typedef struct {
    int x, y, w, h;
} generic_rect_t;

/**
 * @brief Bind a generic texture renderer
 * @param cache Font cache
 * @param renderer Texture renderer implementing the texture_renderer_ops_t interface
 */
void stb_font_bind_texture_renderer(stb_font_cache_t* cache, texture_renderer_t* renderer);

/**
 * @brief Render text to a texture
 * @param cache Font cache
 * @param text UTF-8 string to render
 * @param max_len Maximum length in bytes (use -1 for null-terminated)
 * @param width_out Output width (can be NULL)
 * @param height_out Output height (can be NULL)
 * @return Platform-specific texture (void*), caller must free
 */
void* stb_font_render_to_texture(stb_font_cache_t* cache, 
                                 const char* text, 
                                 int max_len, 
                                 int* width_out, 
                                 int* height_out);

/**
 * @brief Render formatted text to a texture
 * @param cache Font cache
 * @param text UTF-8 string to render
 * @param format Text format
 * @param max_len Maximum length in bytes (use -1 for null-terminated)
 * @param width_out Output width (can be NULL)
 * @param height_out Output height (can be NULL)
 * @return Platform-specific texture (void*), caller must free
 */
void* stb_font_render_formatted_to_texture(stb_font_cache_t* cache, 
                                          const char* text, 
                                          const stb_font_text_format_t* format, 
                                          int max_len, 
                                          int* width_out, 
                                          int* height_out);

/**
 * @brief Pre-rendered text object for generic renderer
 */
typedef struct {
    void* texture;
    texture_renderer_t* renderer;
    int width, height;
} generic_prerendered_text_t;

/**
 * @brief Render text to a prerendered text object
 * @param cache Font cache
 * @param text_out Prerendered text object to populate
 * @param text UTF-8 string to render
 * @param max_len Maximum length in bytes (use -1 for null-terminated)
 */
void stb_font_render_to_object(stb_font_cache_t* cache, 
                              generic_prerendered_text_t* text_out, 
                              const char* text, 
                              int max_len);

/**
 * @brief Render formatted text to a prerendered text object
 * @param cache Font cache
 * @param text_out Prerendered text object to populate
 * @param text UTF-8 string to render
 * @param format Text format
 * @param max_len Maximum length in bytes (use -1 for null-terminated)
 */
void stb_font_render_formatted_to_object(stb_font_cache_t* cache, 
                                        generic_prerendered_text_t* text_out, 
                                        const char* text, 
                                        const stb_font_text_format_t* format, 
                                        int max_len);

/**
 * @brief Draw a prerendered text object
 * @param text Prerendered text object
 * @param x X position
 * @param y Y position
 * @return X position after drawing
 */
int stb_font_draw_prerendered(generic_prerendered_text_t* text, int x, int y);

/**
 * @brief Free texture in prerendered text object
 * @param text Prerendered text object
 */
void stb_font_free_prerendered(generic_prerendered_text_t* text);

#endif /* STB_FONT_TEXTURE_RENDERER_ENABLED */

/*=============================================================================
 * SDL-Specific Functions (Convenience Wrappers)
 *===========================================================================*/

#ifdef STB_FONT_SDL_ENABLED
struct SDL_Renderer;

/**
 * @brief Bind an SDL renderer for rendering
 * @param cache Font cache
 * @param renderer SDL renderer (must be cast from SDL_Renderer*)
 * 
 * This is a convenience function that creates a generic texture renderer
 * adapter for SDL and binds it to the font cache.
 */
void stb_font_sdl_bind_renderer(stb_font_cache_t* cache, void* renderer);

/**
 * @brief Render text to an SDL texture
 * @param cache Font cache
 * @param text UTF-8 string to render
 * @param max_len Maximum length in bytes (use -1 for null-terminated)
 * @param width_out Output width (can be NULL)
 * @param height_out Output height (can be NULL)
 * @return SDL texture (void*), caller must free
 */
void* stb_font_sdl_render_to_texture(stb_font_cache_t* cache, 
                                     const char* text, 
                                     int max_len, 
                                     int* width_out, 
                                     int* height_out);

/**
 * @brief Render formatted text to an SDL texture
 * @param cache Font cache
 * @param text UTF-8 string to render
 * @param format Text format
 * @param max_len Maximum length in bytes (use -1 for null-terminated)
 * @param width_out Output width (can be NULL)
 * @param height_out Output height (can be NULL)
 * @return SDL texture (void*), caller must free
 */
void* stb_font_sdl_render_formatted_to_texture(stb_font_cache_t* cache, 
                                              const char* text, 
                                              const stb_font_text_format_t* format, 
                                              int max_len, 
                                              int* width_out, 
                                              int* height_out);

/**
 * @brief Render text to a prerendered text object
 * @param cache Font cache
 * @param text_out Prerendered text object to populate
 * @param text UTF-8 string to render
 * @param max_len Maximum length in bytes (use -1 for null-terminated)
 */
void stb_font_sdl_render_to_object(stb_font_cache_t* cache, 
                                   stb_prerendered_text_t* text_out, 
                                   const char* text, 
                                   int max_len);

/**
 * @brief Render formatted text to a prerendered text object
 * @param cache Font cache
 * @param text_out Prerendered text object to populate
 * @param text UTF-8 string to render
 * @param format Text format
 * @param max_len Maximum length in bytes (use -1 for null-terminated)
 */
void stb_font_sdl_render_formatted_to_object(stb_font_cache_t* cache, 
                                            stb_prerendered_text_t* text_out, 
                                            const char* text, 
                                            const stb_font_text_format_t* format, 
                                            int max_len);

/**
 * @brief Draw a prerendered text object
 * @param text Prerendered text object
 * @param x X position
 * @param y Y position
 * @return X position after drawing
 */
int stb_font_sdl_draw_prerendered(stb_prerendered_text_t* text, int x, int y);

/**
 * @brief Free texture in prerendered text object
 * @param text Prerendered text object
 */
void stb_font_sdl_free_prerendered(stb_prerendered_text_t* text);
#endif /* STB_FONT_SDL_ENABLED */

/*=============================================================================
 * Utility Functions
 *===========================================================================*/

/**
 * @brief Allocate memory for font data
 * @param mem Memory structure to initialize
 * @param size Number of bytes to allocate
 * @return 0 on success, non-zero on failure
 */
int stb_font_memory_alloc(stb_font_memory_t* mem, size_t size);

/**
 * @brief Free memory owned by a font memory structure
 * @param mem Memory structure to free
 */
void stb_font_memory_free(stb_font_memory_t* mem);

/**
 * @brief Create a text format with specified color
 * @param r Red (0-255)
 * @param g Green (0-255)
 * @param b Blue (0-255)
 * @param a Alpha (0-255, default 255)
 * @return Text format structure
 */
stb_font_text_format_t stb_font_format_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * @brief Create a text format with specified flags
 * @param flags Format flags (STB_FONT_FORMAT_*)
 * @return Text format structure
 */
stb_font_text_format_t stb_font_format_flags(uint8_t flags);

/**
 * @brief Combine two text formats
 * @param dst Destination format (will be modified)
 * @param src Source format
 */
void stb_font_format_combine(stb_font_text_format_t* dst, const stb_font_text_format_t* src);

/**
 * @brief Reset color in a format to white
 * @param fmt Format to reset
 */
void stb_font_format_reset_color(stb_font_text_format_t* fmt);

#ifdef __cplusplus
}
#endif

#endif /* STB_FONT_CACHE_H */
