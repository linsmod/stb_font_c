/**
 * @file stb_font_cache.c
 * @brief C implementation for stb_truetype font rendering library
 * 
 * Public Domain / CC0
 */

#include "stb_font_cache.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* Use stb_truetype implementation */
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

/* Maximum number of fonts in a font list */
#define MAX_FONTS 32
#define MAX_FORMAT_VARIANTS 4

/* UTF-8 utilities */
static int utf8_char_size(const char* s) {
    unsigned char b = (unsigned char)s[0];
    if ((b & 0x80) == 0) return 1;
    if ((b & 0xE0) == 0xC0) return 2;
    if ((b & 0xF0) == 0xE0) return 3;
    if ((b & 0xF8) == 0xF0) return 4;
    return 1;
}

static uint32_t utf8_decode(const char* s, int* char_size) {
    int sz = utf8_char_size(s);
    if (char_size) *char_size = sz;
    
    if (sz == 1) return (uint8_t)s[0];
    if (sz == 2) {
        return ((uint32_t)(s[0] & 0x1F) << 6) | 
               ((uint32_t)(s[1] & 0x3F));
    }
    if (sz == 3) {
        return ((uint32_t)(s[0] & 0x0F) << 12) | 
               ((uint32_t)(s[1] & 0x3F) << 6) | 
               ((uint32_t)(s[2] & 0x3F));
    }
    if (sz == 4) {
        return ((uint32_t)(s[0] & 0x07) << 18) | 
               ((uint32_t)(s[1] & 0x3F) << 12) | 
               ((uint32_t)(s[2] & 0x3F) << 6) | 
               ((uint32_t)(s[3] & 0x3F));
    }
    return (uint8_t)s[0];
}

/* Font entry structure */
typedef struct font_entry {
    stbtt_fontinfo info;
    unsigned char* font_data;
    size_t font_data_size;
    int owned;
    uint8_t format_mask;
    struct font_entry* next;
    struct font_entry* format_variants[MAX_FORMAT_VARIANTS];
} font_entry_t;


typedef struct glyph_cache_entry {
    uint64_t key;
    stb_glyph_t glyph;
    unsigned char* bitmap;
    struct glyph_cache_entry* next;
} glyph_cache_entry_t;

/* Hash table for glyphs */
#define GLYPH_CACHE_SIZE 1024
/* Internal cache structure */
typedef struct {
    font_entry_t* fonts;
    int num_fonts;
    int capacity;
    glyph_cache_entry_t* glyph_cache[GLYPH_CACHE_SIZE];
} cache_internal_t;

/*=============================================================================
 * Core Functions
 *===========================================================================*/

stb_font_cache_t* stb_font_cache_create(const texture_renderer_ops_t* ops, void* context) {
    stb_font_cache_t* cache = (stb_font_cache_t*)calloc(1, sizeof(stb_font_cache_t));
    if (!cache) return NULL;
    
    cache_internal_t* internal = (cache_internal_t*)calloc(1, sizeof(cache_internal_t));
    if (!internal) {
        free(cache);
        return NULL;
    }
    
    internal->fonts = NULL;
    internal->num_fonts = 0;
    internal->capacity = 0;
    
    /* Set defaults */
    cache->face_size = 20;
    cache->tab_width_in_spaces = 8;
    cache->tab_width = 1;
    cache->internal = internal;
    cache->ops = ops;
    cache->ops_ctx = context;
    
    return cache;
}

void stb_font_cache_destroy(stb_font_cache_t* cache) {
    if (!cache) return;
    
    cache_internal_t* internal = (cache_internal_t*)cache->internal;
    /* Free all fonts */
    font_entry_t* font = internal->fonts;
    while (font) {
        font_entry_t* next = font->next;
        
        if (font->owned && font->font_data) {
            free(font->font_data);
        }
        free(font);
        font = next;
    }
    
    free(internal);
    free(cache);
}

void stb_font_cache_set_face_size(stb_font_cache_t* cache, int face_size) {
    if (cache) {
        cache->face_size = face_size;
    }
}

int stb_font_cache_get_row_height(stb_font_cache_t* cache) {
    if (!cache) return 0;
    return (int)(cache->scale * cache->row_size);
}

/*=============================================================================
 * Font Loading
 *===========================================================================*/

static int load_font_common(stb_font_cache_t* cache, 
                          unsigned char* ttf_buffer, 
                          size_t ttf_buffer_len, 
                          int index, 
                          int owned, 
                          uint8_t format_mask,
                          font_entry_t** out_font) {
    cache_internal_t* internal = (cache_internal_t*)cache->internal;
    
    font_entry_t* font = (font_entry_t*)calloc(1, sizeof(font_entry_t));
    if (!font) return -1;
    
    font->font_data = ttf_buffer;
    font->font_data_size = ttf_buffer_len;
    font->owned = owned;
    font->format_mask = format_mask;
    font->next = NULL;
    
    int offset = stbtt_GetFontOffsetForIndex(ttf_buffer, index);
    if (!stbtt_InitFont(&font->info, ttf_buffer, offset)) {
        free(font);
        return -1;
    }
    
    if (!internal->fonts) {
        /* First font - calculate metrics */
        internal->fonts = font;
        
        stbtt_GetFontVMetrics(&font->info, &cache->ascent, &cache->descent, &cache->line_gap);
        cache->scale = stbtt_ScaleForPixelHeight(&font->info, cache->face_size);
        cache->baseline = (int)(cache->ascent * cache->scale);
        cache->row_size = cache->ascent - cache->descent + cache->line_gap;
        
        cache->strikethrough_thickness = cache->face_size / 20.0f;
        if (cache->strikethrough_thickness < 1.0f) cache->strikethrough_thickness = 1.0f;
        cache->strikethrough_position = cache->baseline * 0.75f - cache->strikethrough_thickness / 2.0f;
        cache->underline_thickness = cache->strikethrough_thickness;
        cache->underline_position = cache->baseline + cache->underline_thickness;
        
        /* Calculate tab width */
        int w, h;
        stb_font_get_text_size(cache, "                                                                                                                                ", 
                              cache->tab_width_in_spaces, &w, &h);
        cache->tab_width = w;
        if (cache->tab_width < 1) cache->tab_width = 1;
        
        if (out_font) *out_font = font;
    } else {
        /* Additional font - add to chain */
        font_entry_t* f = internal->fonts;
        while (f->next) f = f->next;
        f->next = font;
        
        if (out_font) *out_font = font;
    }
    
    internal->num_fonts++;
    return 0;
}

int stb_font_load(stb_font_cache_t* cache, 
                  const unsigned char* ttf_buffer, 
                  size_t ttf_buffer_len, 
                  int index) {
    return load_font_common(cache, (unsigned char*)ttf_buffer, ttf_buffer_len, index, 0, 0, NULL);
}

int stb_font_load_managed(stb_font_cache_t* cache, 
                         stb_font_memory_t* memory, 
                         int index) {
    font_entry_t* font;
    int ret = load_font_common(cache, memory->data, memory->size, index, 1, 0, &font);
    if (ret == 0 && font) {
        memory->owned = 0; /* Transfer ownership */
    }
    return ret;
}

int stb_font_add(stb_font_cache_t* cache, 
                 const unsigned char* ttf_buffer, 
                 size_t ttf_buffer_len, 
                 int index) {
    return load_font_common(cache, (unsigned char*)ttf_buffer, ttf_buffer_len, index, 0, 0, NULL);
}

int stb_font_add_managed(stb_font_cache_t* cache, 
                        stb_font_memory_t* memory, 
                        int index) {
    font_entry_t* font;
    int ret = load_font_common(cache, memory->data, memory->size, index, 1, 0, &font);
    if (ret == 0 && font) {
        memory->owned = 0; /* Transfer ownership */
    }
    return ret;
}

static font_entry_t* find_format_variant(font_entry_t* font, uint8_t format_mask) {
    for (int i = 0; i < MAX_FORMAT_VARIANTS; i++) {
        if (font->format_variants[i] && font->format_variants[i]->format_mask == format_mask) {
            return font->format_variants[i];
        }
    }
    return NULL;
}

int stb_font_add_format_font(stb_font_cache_t* cache, 
                            uint8_t format_mask, 
                            const unsigned char* ttf_buffer, 
                            size_t ttf_buffer_len, 
                            int index) {
    cache_internal_t* internal = (cache_internal_t*)cache->internal;
    font_entry_t* primary_font = internal->fonts;
    if (!primary_font) return -1;
    
    /* Check if variant already exists */
    if (find_format_variant(primary_font, format_mask)) {
        return -2; /* Already exists */
    }
    
    font_entry_t* variant = NULL;
    int ret = load_font_common(cache, (unsigned char*)ttf_buffer, ttf_buffer_len, index, 0, format_mask, &variant);
    if (ret == 0 && variant) {
        /* Add to primary font's variants */
        for (int i = 0; i < MAX_FORMAT_VARIANTS; i++) {
            if (!primary_font->format_variants[i]) {
                primary_font->format_variants[i] = variant;
                break;
            }
        }
    }
    
    return ret;
}

int stb_font_add_format_font_managed(stb_font_cache_t* cache, 
                                    uint8_t format_mask, 
                                    stb_font_memory_t* memory, 
                                    int index) {
    cache_internal_t* internal = (cache_internal_t*)cache->internal;
    font_entry_t* primary_font = internal->fonts;
    if (!primary_font) return -1;
    
    font_entry_t* variant = NULL;
    int ret = load_font_common(cache, memory->data, memory->size, index, 1, format_mask, &variant);
    if (ret == 0 && variant) {
        memory->owned = 0; /* Transfer ownership */
        
        /* Add to primary font's variants */
        for (int i = 0; i < MAX_FORMAT_VARIANTS; i++) {
            if (!primary_font->format_variants[i]) {
                primary_font->format_variants[i] = variant;
                break;
            }
        }
    }
    
    return ret;
}

/*=============================================================================
 * Text Measurement
 *===========================================================================*/

static font_entry_t* find_font_for_codepoint(cache_internal_t* internal, uint32_t codepoint, uint8_t format_mask, int* glyph_idx) {
    (void)format_mask; /* Currently unused, for future format variant support */
    font_entry_t* font = internal->fonts;
    
    while (font) {
        int idx = stbtt_FindGlyphIndex(&font->info, codepoint);
        if (idx > 0) {
            *glyph_idx = idx;
            return font;
        }
        font = font->next;
    }
    
    /* Not found, return primary font */
    *glyph_idx = 0;
    return internal->fonts;
}

void stb_font_get_text_size(stb_font_cache_t* cache, 
                           const char* text, 
                           int max_len, 
                           int* width_out, 
                           int* height_out) {
    if (!cache || !text) {
        if (width_out) *width_out = 0;
        if (height_out) *height_out = 0;
        return;
    }
    
    cache_internal_t* internal = (cache_internal_t*)cache->internal;
    
    int x = 0;
    int y = 0;
    int max_x = 0;
    int max_y = 0;
    uint32_t prev_char = 0;
    
    int len = max_len;
    if (len < 0) len = (int)strlen(text);
    
    for (int pos = 0; pos < len; ) {
        int char_size = 0;
        uint32_t c = utf8_decode(text + pos, &char_size);
        
        if (c == '\n') {
            x = 0;
            y += (int)(cache->scale * cache->row_size);
            pos += char_size;
            prev_char = 0;
            continue;
        }
        
        if (c == '\t') {
            int n_tabs = x / cache->tab_width;
            x = (n_tabs + 1) * cache->tab_width;
            pos += char_size;
            prev_char = 0;
            continue;
        }
        
        /* Get glyph metrics */
        int glyph_idx = 0;
        font_entry_t* font = find_font_for_codepoint(internal, c, 0, &glyph_idx);
        
        int advance, lsb;
        stbtt_GetGlyphHMetrics(&font->info, glyph_idx, &advance, &lsb);
        
        if (prev_char) {
            int kerning = stbtt_GetGlyphKernAdvance(&font->info, prev_char, glyph_idx);
            x += (int)(cache->scale * kerning);
        }
        
        x += (int)(cache->scale * advance);
        
        if (x > max_x) max_x = x;
        
        prev_char = c;
        pos += char_size;
    }
    
    max_y = y + (int)(cache->scale * cache->row_size);
    
    if (width_out) *width_out = max_x;
    if (height_out) *height_out = max_y;
}

void stb_font_get_formatted_text_size(stb_font_cache_t* cache, 
                                     const char* text, 
                                     const stb_font_text_format_t* format, 
                                     int max_len, 
                                     int* width_out, 
                                     int* height_out) {
    (void)format; /* Currently unused, for future format support */
    /* For now, formatted text size is the same as plain text */
    /* TODO: Account for bold/italic variations */
    stb_font_get_text_size(cache, text, max_len, width_out, height_out);
}

/*=============================================================================
 * Generic Texture-Based Renderer Functions
 *===========================================================================*/

/* Key generation */
static uint64_t glyph_key(uint32_t codepoint, uint8_t format, uint16_t font_idx) {
    return ((uint64_t)codepoint) | ((uint64_t)format << 32) | ((uint64_t)font_idx << 40);
}

static uint32_t glyph_cache_hash(uint64_t key) {
    return (uint32_t)(key % GLYPH_CACHE_SIZE);
}

/* Helper function to convert RGBA data to platform surface */
static void* convert_rgba_to_surface(stb_font_cache_t* cache, 
                                    unsigned char* rgba, int width, int height, int pitch) {
    if (!cache->ops_ctx) return NULL;
    
    if (cache->ops->create_surface_from_rgba) {
        return cache->ops->create_surface_from_rgba(cache->ops_ctx, rgba, width, height, pitch);
    }
    
    /* Fallback: create minimal surface representation */
    fprintf(stderr, "Warning: renderer does not support create_surface_from_rgba\n");
    return NULL;
}

/* Helper function to destroy surface appropriately */
static void destroy_surface(stb_font_cache_t* cache, void* surface) {
    if (!cache->ops_ctx) return;
    
    texture_renderer_ops_t* ops = (texture_renderer_ops_t*)cache->ops;
    if (ops->free_surface) {
        ops->free_surface(cache->ops_ctx, surface);
    }
    /* If free_surface is not provided, we assume the surface is managed by create_texture_from_surface */
}

static stb_glyph_t* get_or_create_glyph(stb_font_cache_t* cache, 
                                       uint32_t codepoint, 
                                       uint8_t format, 
                                       font_entry_t** out_font) {
    cache_internal_t* internal = (cache_internal_t*)cache->internal;
    
    if (!cache->ops_ctx) return NULL;
    
    texture_renderer_ops_t* ops = (texture_renderer_ops_t*)cache->ops;
    
    /* Find font */
    int glyph_idx = 0;
    font_entry_t* font = find_font_for_codepoint(internal, codepoint, format, &glyph_idx);
    if (out_font) *out_font = font;
    
    if (glyph_idx == 0) return NULL;
    
    /* Check cache */
    uint64_t key = glyph_key(codepoint, format, 0);
    uint32_t hash = glyph_cache_hash(key);
    glyph_cache_entry_t* entry = internal->glyph_cache[hash];
    
    while (entry) {
        if (entry->key == key) {
            return &entry->glyph;
        }
        entry = entry->next;
    }
    
    /* Create new glyph */
    entry = (glyph_cache_entry_t*)calloc(1, sizeof(glyph_cache_entry_t));
    if (!entry) return NULL;
    
    entry->key = key;
    
    /* Get glyph metrics */
    int advance, lsb;
    int x0, y0, x1, y1;
    stbtt_GetGlyphHMetrics(&font->info, glyph_idx, &advance, &lsb);
    stbtt_GetGlyphBitmapBox(&font->info, glyph_idx, cache->scale, cache->scale, &x0, &y0, &x1, &y1);
    
    int w = x1 - x0;
    int h = y1 - y0;
    
    entry->glyph.advance = (int16_t)(cache->scale * advance);
    entry->glyph.left_side_bearing = (int16_t)lsb;
    entry->glyph.font_idx = 0;
    entry->glyph.padding = 0;
    entry->glyph.width = (int16_t)w;
    entry->glyph.height = (int16_t)h;
    entry->glyph.x_offset = (int16_t)x0;
    entry->glyph.y_offset = (int16_t)y0;
    entry->glyph.texture = NULL;
    
    /* Render glyph bitmap */
    if (w > 0 && h > 0) {
        unsigned char* bitmap = (unsigned char*)malloc(w * h);
        stbtt_MakeGlyphBitmap(&font->info, bitmap, w, h, w, cache->scale, cache->scale, glyph_idx);
        
        /* Convert to RGBA */
        unsigned char* rgba = (unsigned char*)malloc(w * h * 4);
        for (int i = 0; i < w * h; i++) {
            rgba[i * 4 + 0] = 255;  /* R */
            rgba[i * 4 + 1] = 255;  /* G */
            rgba[i * 4 + 2] = 255;  /* B */
            rgba[i * 4 + 3] = bitmap[i]; /* A */
        }
        free(bitmap);
        
        /* Create texture through renderer interface */
        void* surface = convert_rgba_to_surface(cache, rgba, w, h, w * 4);
        void* texture = NULL;
        
        if (surface) {
            if (ops->create_texture_from_surface) {
                texture = ops->create_texture_from_surface(cache->ops_ctx, surface);
            }
            
            if (texture) {
                /* Set blend mode if available */
                if (ops->set_texture_blend_mode) {
                    ops->set_texture_blend_mode(cache->ops_ctx, texture, 1); /* BLEND */
                }
                
                entry->glyph.texture = texture;
            } else if (ops->get_error) {
                char error_buf[256];
                ops->get_error(cache->ops_ctx, error_buf, sizeof(error_buf));
                fprintf(stderr, "Failed to create texture for glyph %c (codepoint %u): %s\n",
                        (char)codepoint, codepoint, error_buf);
            }
            
            destroy_surface(cache, surface);
        }
        
        free(rgba);
    }
    
    /* Add to cache */
    entry->next = internal->glyph_cache[hash];
    internal->glyph_cache[hash] = entry;
    
    return &entry->glyph;
}

static int draw_text_worker(stb_font_cache_t* cache, 
                          int x, int y, 
                          const char* text, 
                          int max_len,
                          const stb_font_text_format_t* format,
                          int is_drawing) {
    if (!cache || !text) return x;
    
    cache_internal_t* internal = (cache_internal_t*)cache->internal;
    
    void* ops_ctx = cache->ops_ctx;
    
    int len = max_len;
    if (len < 0) len = (int)strlen(text);
    
    uint32_t prev_char = 0;
    int pos = 0;
    
    while (pos < len) {
        int char_size = 0;
        uint32_t c = utf8_decode(text + pos, &char_size);
        
        if (c == '\n') {
            x = 0;
            y += (int)(cache->scale * cache->row_size);
            pos += char_size;
            prev_char = 0;
            continue;
        }
        
        if (c == '\t') {
            int n_tabs = x / cache->tab_width;
            x = (n_tabs + 1) * cache->tab_width;
            pos += char_size;
            prev_char = 0;
            continue;
        }
        
        /* Get or create glyph */
        uint8_t fmt = format ? format->format : 0;
        stb_glyph_t* glyph = get_or_create_glyph(cache, c, fmt, NULL);

        if (glyph && is_drawing) {
            /* Apply kerning */
            font_entry_t* font = find_font_for_codepoint(internal, c, fmt, &(int){0});
            if (prev_char && font) {
                int glyph_idx = 0;
                find_font_for_codepoint(internal, c, fmt, &glyph_idx);
                int kerning = stbtt_GetGlyphKernAdvance(&font->info, prev_char, glyph_idx);
                x += (int)(cache->scale * kerning);
            }
            
            int draw_x = x + glyph->x_offset;
            int draw_y = y + glyph->y_offset + cache->baseline;

            /* Set color if specified and renderer supports it */
            if (format && (format->flags & STB_FONT_FORMAT_COLOR_SET) && 
                cache->ops->set_texture_color_mod && glyph->texture) {
                cache->ops->set_texture_color_mod(ops_ctx, glyph->texture,
                                                   format->r, format->g, format->b);
            }

            /* Render the glyph */
            if (cache->ops->render_copy && glyph->texture) {
                cache->ops->render_copy(ops_ctx, glyph->texture, draw_x, draw_y, glyph->width, glyph->height);
            }
            
            /* Draw underline */
            if (format && (format->format & STB_FONT_FORMAT_UNDERLINE)) {
                if (cache->ops->set_render_draw_color && cache->ops->render_fill_rect) {
                    cache->ops->set_render_draw_color(ops_ctx, 
                                                      format->r, format->g, format->b, format->a);
                    cache->ops->render_fill_rect(ops_ctx, x, 
                    (int)(y + cache->underline_position),
                    glyph->width + (int)cache->underline_thickness,
                    (int)cache->underline_thickness);
                }
            }
            
            /* Draw strikethrough */
            if (format && (format->format & STB_FONT_FORMAT_STRIKETHROUGH)) {
                if (cache->ops->set_render_draw_color && cache->ops->render_fill_rect) {
                    cache->ops->set_render_draw_color(ops_ctx,
                                                      format->r, format->g, format->b, format->a);
                    cache->ops->render_fill_rect(ops_ctx, x,
                    (int)(y + cache->strikethrough_position),
                    glyph->width + (int)cache->strikethrough_thickness,
                    (int)cache->strikethrough_thickness);
                }
            }
            
            /* Reset color if we modified it */
            if (format && (format->flags & STB_FONT_FORMAT_COLOR_SET) && 
                cache->ops->set_texture_color_mod && glyph->texture) {
                cache->ops->set_texture_color_mod(ops_ctx, glyph->texture, 255, 255, 255);
            }
            
            x += glyph->advance;
            prev_char = c;
        }
        
        pos += char_size;
    }
    
    return x;
}

int stb_font_draw_text(stb_font_cache_t* cache, 
                      int x, int y, 
                      const char* text, 
                      int max_len) {
    return draw_text_worker(cache, x, y, text, max_len, NULL, 1);
}

int stb_font_draw_text_formatted(stb_font_cache_t* cache, 
                                int x, int y, 
                                const char* text, 
                                const stb_font_text_format_t* format, 
                                int max_len) {
    return draw_text_worker(cache, x, y, text, max_len, format, 1);
}

/* Generic texture-based rendering functions */
static void* render_to_texture_generic(stb_font_cache_t* cache,
                                      const char* text,
                                      const stb_font_text_format_t* format,
                                      int max_len,
                                      int* width_out,
                                      int* height_out) {
    void* ops_ctx = cache->ops_ctx;
    
    /* Get text size */
    int width, height;
    if (format) {
        stb_font_get_formatted_text_size(cache, text, format, max_len, &width, &height);
    } else {
        stb_font_get_text_size(cache, text, max_len, &width, &height);
    }
    
    if (width <= 0 || height <= 0) {
        if (width_out) *width_out = 0;
        if (height_out) *height_out = 0;
        return NULL;
    }
    
    /* Create render target texture if supported */
    void* target = NULL;
    void* old_target = NULL;
    
    if (cache->ops->create_texture && cache->ops->set_render_target && cache->ops->get_render_target) {
        target = cache->ops->create_texture(ops_ctx, width, height, 0); /* Format TBD */
        if (target) {
            old_target = cache->ops->get_render_target(ops_ctx);
            cache->ops->set_render_target(ops_ctx, target);
        }
    }
    
    if (!target) {
        if (width_out) *width_out = 0;
        if (height_out) *height_out = 0;
        return NULL;
    }
    
    /* Clear the texture */
    if (cache->ops->set_render_draw_color && cache->ops->render_clear) {
        cache->ops->set_render_draw_color(ops_ctx, 255, 255, 255, 0);
        cache->ops->render_clear(ops_ctx);
    } else if (cache->ops->set_render_draw_blend_mode && cache->ops->render_fill_rect) {
        cache->ops->set_render_draw_blend_mode(ops_ctx, 0); /* NONE */
        cache->ops->set_render_draw_color(ops_ctx, 255, 255, 255, 0);
        cache->ops->render_fill_rect(ops_ctx, 0, 0, width, height);
        cache->ops->set_render_draw_blend_mode(ops_ctx, 1); /* BLEND */
    }
    
    /* Draw text */
    draw_text_worker(cache, 0, 0, text, max_len, format, 1);
    
    /* Restore previous render target */
    if (old_target && cache->ops->set_render_target) {
        cache->ops->set_render_target(ops_ctx, old_target);
    }
    
    if (width_out) *width_out = width;
    if (height_out) *height_out = height;
    
    return target;
}

void* stb_font_render_to_texture(stb_font_cache_t* cache, 
                                 const char* text, 
                                 int max_len, 
                                 int* width_out, 
                                 int* height_out) {
    return render_to_texture_generic(cache, text, NULL, max_len, width_out, height_out);
}

void* stb_font_render_formatted_to_texture(stb_font_cache_t* cache, 
                                          const char* text, 
                                          const stb_font_text_format_t* format, 
                                          int max_len, 
                                          int* width_out, 
                                          int* height_out) {
    return render_to_texture_generic(cache, text, format, max_len, width_out, height_out);
}

void* stb_font_render_to_object(stb_font_cache_t* cache, 
                              const char* text, 
                              int max_len,
                             int* width_out, int* height_out) {
    
    return render_to_texture_generic(cache, text, NULL, max_len, width_out, height_out);
}
void* stb_font_render_formatted_to_object(stb_font_cache_t* cache, 
                                        const char* text, 
                                        const stb_font_text_format_t* format, 
                                        int max_len, 
                                        int* width_out, int* height_out) {
    
    return render_to_texture_generic(cache, text, format, max_len, width_out, height_out);
}


/*=============================================================================
 * Utility Functions
 *===========================================================================*/

int stb_font_memory_alloc(stb_font_memory_t* mem, size_t size) {
    if (!mem) return -1;
    
    mem->data = (unsigned char*)malloc(size);
    if (!mem->data) return -1;
    
    mem->size = size;
    mem->owned = 1;
    
    return 0;
}

void stb_font_memory_free(stb_font_memory_t* mem) {
    if (!mem) return;
    
    if (mem->owned && mem->data) {
        free(mem->data);
        mem->data = NULL;
        mem->size = 0;
    }
}

stb_font_text_format_t stb_font_format_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    stb_font_text_format_t fmt;
    fmt.r = r;
    fmt.g = g;
    fmt.b = b;
    fmt.a = a;
    fmt.format = 0;
    fmt.flags = STB_FONT_FORMAT_COLOR_SET;
    return fmt;
}

stb_font_text_format_t stb_font_format_flags(uint8_t flags) {
    stb_font_text_format_t fmt;
    fmt.r = 255;
    fmt.g = 255;
    fmt.b = 255;
    fmt.a = 255;
    fmt.format = flags;
    fmt.flags = 0;
    return fmt;
}

void stb_font_format_combine(stb_font_text_format_t* dst, const stb_font_text_format_t* src) {
    if (!dst || !src) return;
    
    dst->format |= src->format;
    dst->flags |= src->flags;
    
    /* Blend colors multiplicatively */
    if (src->flags & STB_FONT_FORMAT_COLOR_SET) {
        dst->r = (uint8_t)(255 * (dst->r / 255.0) * (src->r / 255.0));
        dst->g = (uint8_t)(255 * (dst->g / 255.0) * (src->g / 255.0));
        dst->b = (uint8_t)(255 * (dst->b / 255.0) * (src->b / 255.0));
        dst->a = (uint8_t)(255 * (dst->a / 255.0) * (src->a / 255.0));
    }
}

void stb_font_format_reset_color(stb_font_text_format_t* fmt) {
    if (!fmt) return;
    
    fmt->r = 255;
    fmt->g = 255;
    fmt->b = 255;
    fmt->a = 255;
    fmt->flags &= ~STB_FONT_FORMAT_COLOR_SET;
}
