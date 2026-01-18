#include <SDL2/SDL.h>
#include <wchar.h>
#include "stb_font_cache.h"
/* SDL-specific renderer implementation */
typedef struct {
    SDL_Renderer* sdl_renderer;
} sdl_renderer_data_t;

/* SDL adapter functions */
static void* sdl_create_texture_from_surface(void* renderer, void* surface) {
    
    SDL_Surface* sdlsurf = (SDL_Surface*)surface;
    return SDL_CreateTextureFromSurface((SDL_Renderer*)renderer, sdlsurf);
}

static void* sdl_create_texture(void* renderer, int width, int height, int format) {
    (void)format; /* Format parameter is currently unused */
    
    Uint32 sdl_format = SDL_PIXELFORMAT_RGBA8888;
    return SDL_CreateTexture((SDL_Renderer*)renderer, sdl_format, SDL_TEXTUREACCESS_TARGET, width, height);
}

static void sdl_destroy_texture(void* renderer, void* texture) {
    (void)renderer; /* Parameter unused but required for interface */
    SDL_DestroyTexture((SDL_Texture*)texture);
}

static void sdl_set_texture_blend_mode(void* renderer, void* texture, int blend_mode) {
    (void)renderer; /* Parameter unused but required for interface */
    SDL_SetTextureBlendMode((SDL_Texture*)texture, 
                           blend_mode ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
}

static void sdl_set_texture_color_mod(void* renderer, void* texture, uint8_t r, uint8_t g, uint8_t b) {
    (void)renderer; /* Parameter unused but required for interface */
    SDL_SetTextureColorMod((SDL_Texture*)texture, r, g, b);
}

static void* sdl_create_surface_from_rgba(void* renderer, unsigned char* rgba, int width, int height, int pitch) {
    (void)renderer; /* Parameter unused but required for interface */
    return SDL_CreateRGBSurfaceFrom(rgba, width, height, 32, pitch,
                                   0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
}

static void sdl_free_surface(void* renderer, void* surface) {
    (void)renderer; /* Parameter unused but required for interface */
    SDL_FreeSurface((SDL_Surface*)surface);
}

static void sdl_render_copy(void* renderer, void* texture, int dst_rect_x, int dst_rect_y, int width, int height) {
    
    const SDL_Rect* srcrect = (const SDL_Rect*)NULL;
    SDL_Rect dstrect = {dst_rect_x, dst_rect_y, width, height};
    SDL_RenderCopy((SDL_Renderer*)renderer, (SDL_Texture*)texture, srcrect, &dstrect);
}

static void sdl_set_render_target(void* renderer, void* target) {
    
    SDL_SetRenderTarget((SDL_Renderer*)renderer, (SDL_Texture*)target);
}

static void* sdl_get_render_target(void* renderer) {
    
    return SDL_GetRenderTarget((SDL_Renderer*)renderer);
}

static void sdl_set_render_draw_color(void* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    
    SDL_SetRenderDrawColor((SDL_Renderer*)renderer, r, g, b, a);
}

static void sdl_set_render_draw_blend_mode(void* renderer, int blend_mode) {
    
    SDL_SetRenderDrawBlendMode((SDL_Renderer*)renderer, 
                               blend_mode ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
}

static void sdl_render_fill_rect(void* renderer, int rect_x, int rect_y, int width, int height) {
    
    SDL_Rect rect = {rect_x, rect_y, width, height};
    SDL_RenderFillRect((SDL_Renderer*)renderer, &rect);
}

static void sdl_render_clear(void* renderer) {
    
    SDL_RenderClear((SDL_Renderer*)renderer);
}

static int sdl_get_error(void* renderer, char* buffer, int buffer_size) {
    (void)renderer; /* Parameter unused but required for interface */
    const char* err = SDL_GetError();
    strncpy(buffer, err, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return (int)strlen(buffer);
}

/* Create SDL texture renderer */
texture_renderer_ops_t* stb_font_create_renderer_funcs(SDL_Renderer* sdl_renderer) {
    texture_renderer_ops_t* ops = (texture_renderer_ops_t*)malloc(sizeof(texture_renderer_ops_t));
    ops->create_texture_from_surface = sdl_create_texture_from_surface;
    ops->create_texture = sdl_create_texture;
    ops->destroy_texture = sdl_destroy_texture;
    ops->set_texture_blend_mode = sdl_set_texture_blend_mode;
    ops->set_texture_color_mod = sdl_set_texture_color_mod;
    ops->create_surface_from_rgba = sdl_create_surface_from_rgba;
    ops->free_surface = sdl_free_surface;
    ops->render_copy = sdl_render_copy;
    ops->set_render_target = sdl_set_render_target;
    ops->get_render_target = sdl_get_render_target;
    ops->set_render_draw_color = sdl_set_render_draw_color;
    ops->set_render_draw_blend_mode = sdl_set_render_draw_blend_mode;
    ops->render_fill_rect = sdl_render_fill_rect;
    ops->render_clear = sdl_render_clear;
    ops->get_error = sdl_get_error;
    return ops;
}

/* SDL-specific wrapper functions using the generic interface */
void* stb_font_sdl_render_to_texture(stb_font_cache_t* cache, 
                                     const char* text, 
                                     int max_len, 
                                     int* width_out, 
                                     int* height_out) {
    return stb_font_render_to_texture(cache, text, max_len, width_out, height_out);
}

void* stb_font_sdl_render_formatted_to_texture(stb_font_cache_t* cache, 
                                              const char* text, 
                                              const stb_font_text_format_t* format, 
                                              int max_len, 
                                              int* width_out, 
                                              int* height_out) {
    return stb_font_render_formatted_to_texture(cache, text, format, max_len, width_out, height_out);
}

void* stb_font_sdl_render_to_object(stb_font_cache_t* cache, 
                                   const char* text, 
                                   int max_len, 
                                   int* width_out, 
                                   int* height_out) {
    return stb_font_render_to_object(cache, text, max_len, width_out, height_out);
}

void* stb_font_sdl_render_formatted_to_object(stb_font_cache_t* cache, 
                                            const char* text, 
                                            const stb_font_text_format_t* format, 
                                            int max_len, 
                                            int* width_out, 
                                            int* height_out) {
    
    return stb_font_render_formatted_to_object(cache, text, format, max_len, width_out, height_out);
}