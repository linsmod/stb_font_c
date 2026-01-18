/**
 * @file stb_font_simple.c
 * @brief Implementation of simplified wrapper for stb_font_cache
 * 
 * Public Domain / CC0
 */

#include "stb_font_simple.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Default font directory (NULL means no default) */
static const char* g_font_directory = NULL;

/*=============================================================================
 * Helper Functions
 *===========================================================================*/

/**
 * @brief Build full path from filename and default directory
 * @param filename Font filename
 * @param output Buffer to store full path
 * @param output_size Size of output buffer
 */
static void build_full_path(const char* filename, char* output, size_t output_size) {
    if (g_font_directory && filename[0] != '/') {
        /* Relative path with directory prefix */
        snprintf(output, output_size, "%s/%s", g_font_directory, filename);
    } else {
        /* Absolute path or no directory set */
        strncpy(output, filename, output_size - 1);
        output[output_size - 1] = '\0';
    }
}

/**
 * @brief Read a font file into memory
 * @param filename Font file path
 * @param memory Memory structure to fill
 * @return 0 on success, non-zero on failure
 */
static int read_font_file(const char* filename, stb_font_memory_t* memory) {
    char full_path[512];
    build_full_path(filename, full_path, sizeof(full_path));
    
    FILE* fp = fopen(full_path, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open font file: %s\n", full_path);
        return -1;
    }
    
    /* Get file size */
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fprintf(stderr, "Invalid file size: %s\n", full_path);
        fclose(fp);
        return -1;
    }
    
    /* Allocate memory */
    if (stb_font_memory_alloc(memory, (size_t)file_size) != 0) {
        fprintf(stderr, "Failed to allocate memory for font: %s\n", full_path);
        fclose(fp);
        return -1;
    }
    
    /* Read file */
    size_t read = fread(memory->data, 1, (size_t)file_size, fp);
    fclose(fp);
    
    if (read != (size_t)file_size) {
        fprintf(stderr, "Failed to read font file: %s (read %zu of %ld bytes)\n", 
                full_path, read, file_size);
        stb_font_memory_free(memory);
        return -1;
    }
    
    return 0;
}

/*=============================================================================
 * Simplified Font Loading - Direct File Loading
 *===========================================================================*/

int stb_font_load_file(stb_font_cache_t* cache, const char* filename, int index) {
    if (!cache || !filename) {
        return -1;
    }
    
    stb_font_memory_t memory = {0};
    if (read_font_file(filename, &memory) != 0) {
        return -1;
    }
    
    int ret = stb_font_load_managed(cache, &memory, index);
    if (ret != 0) {
        stb_font_memory_free(&memory);
        return -1;
    }
    
    return 0;
}

int stb_font_add_file(stb_font_cache_t* cache, const char* filename, int index) {
    if (!cache || !filename) {
        return -1;
    }
    
    stb_font_memory_t memory = {0};
    if (read_font_file(filename, &memory) != 0) {
        return -1;
    }
    
    int ret = stb_font_add_managed(cache, &memory, index);
    if (ret != 0) {
        stb_font_memory_free(&memory);
        return -1;
    }
    
    return 0;
}

int stb_font_add_format_file(stb_font_cache_t* cache, 
                             uint8_t format_mask, 
                             const char* filename, 
                             int index) {
    if (!cache || !filename) {
        return -1;
    }
    
    stb_font_memory_t memory = {0};
    if (read_font_file(filename, &memory) != 0) {
        return -1;
    }
    
    int ret = stb_font_add_format_font_managed(cache, format_mask, &memory, index);
    if (ret != 0) {
        stb_font_memory_free(&memory);
        return -1;
    }
    
    return 0;
}

/*=============================================================================
 * Batch Font Loading
 *===========================================================================*/

int stb_font_load_multiple(stb_font_cache_t* cache, 
                          const stb_font_config_t* configs, 
                          int count) {
    if (!cache || !configs || count <= 0) {
        return -1;
    }
    
    int loaded = 0;
    
    for (int i = 0; i < count; i++) {
        const stb_font_config_t* cfg = &configs[i];
        
        if (!cfg->filename) {
            fprintf(stderr, "Skipping NULL filename at index %d\n", i);
            continue;
        }
        
        int ret;
        if (i == 0 && cfg->format_mask == 0) {
            /* First font with no format - use as main font */
            ret = stb_font_load_file(cache, cfg->filename, cfg->index);
        } else if (cfg->format_mask != 0) {
            /* Format variant */
            ret = stb_font_add_format_file(cache, cfg->format_mask, 
                                            cfg->filename, cfg->index);
        } else {
            /* Fallback font */
            ret = stb_font_add_file(cache, cfg->filename, cfg->index);
        }
        
        if (ret == 0) {
            loaded++;
        } else {
            fprintf(stderr, "Failed to load font: %s\n", cfg->filename);
        }
    }
    
    return loaded;
}

/*=============================================================================
 * One-Step Initialization
 *===========================================================================*/

stb_font_cache_t* stb_font_cache_init_simple(const texture_renderer_ops_t* renderer_funcs,
                                              void* renderer_context,
                                              int font_size,
                                              const char* main_font) {
    if (!renderer_funcs || !main_font) {
        return NULL;
    }
    
    /* Create cache */
    stb_font_cache_t* cache = stb_font_cache_create(renderer_funcs, renderer_context);
    if (!cache) {
        return NULL;
    }
    
    /* Set font size */
    stb_font_cache_set_face_size(cache, font_size);
    
    /* Load main font */
    if (stb_font_load_file(cache, main_font, 0) != 0) {
        stb_font_cache_destroy(cache);
        return NULL;
    }
    
    return cache;
}

stb_font_cache_t* stb_font_cache_init_multiple(const texture_renderer_ops_t* renderer_funcs,
                                                void* renderer_context,
                                                int font_size,
                                                const stb_font_config_t* configs,
                                                int count) {
    if (!renderer_funcs || !configs || count <= 0) {
        return NULL;
    }
    
    /* Create cache */
    stb_font_cache_t* cache = stb_font_cache_create(renderer_funcs, renderer_context);
    if (!cache) {
        return NULL;
    }
    
    /* Set font size */
    stb_font_cache_set_face_size(cache, font_size);
    
    /* Load fonts */
    int loaded = stb_font_load_multiple(cache, configs, count);
    if (loaded <= 0) {
        fprintf(stderr, "Failed to load any fonts\n");
        stb_font_cache_destroy(cache);
        return NULL;
    }
    
    return cache;
}

/*=============================================================================
 * Convenience Loading by Script/Language
 *===========================================================================*/

int stb_font_load_family(stb_font_cache_t* cache, const stb_font_family_t* family) {
    if (!cache || !family) {
        return -1;
    }
    
    /* Load regular font (must exist) */
    if (!family->regular) {
        fprintf(stderr, "Regular font is required\n");
        return -1;
    }
    
    if (stb_font_load_file(cache, family->regular, 0) != 0) {
        return -1;
    }
    
    /* Load optional variants */
    if (family->bold) {
        stb_font_add_format_file(cache, STB_FONT_FORMAT_BOLD, family->bold, 0);
    }
    
    if (family->italic) {
        stb_font_add_format_file(cache, STB_FONT_FORMAT_ITALIC, family->italic, 0);
    }
    
    if (family->bold_italic) {
        stb_font_add_format_file(cache, 
                                STB_FONT_FORMAT_BOLD | STB_FONT_FORMAT_ITALIC, 
                                family->bold_italic, 0);
    }
    
    return 0;
}

int stb_font_add_family(stb_font_cache_t* cache, const stb_font_family_t* family) {
    if (!cache || !family) {
        return -1;
    }
    
    /* Add regular font as fallback */
    if (!family->regular) {
        fprintf(stderr, "Regular font is required\n");
        return -1;
    }
    
    if (stb_font_add_file(cache, family->regular, 0) != 0) {
        return -1;
    }
    
    /* Add optional variants */
    if (family->bold) {
        stb_font_add_format_file(cache, STB_FONT_FORMAT_BOLD, family->bold, 0);
    }
    
    if (family->italic) {
        stb_font_add_format_file(cache, STB_FONT_FORMAT_ITALIC, family->italic, 0);
    }
    
    if (family->bold_italic) {
        stb_font_add_format_file(cache, 
                                STB_FONT_FORMAT_BOLD | STB_FONT_FORMAT_ITALIC, 
                                family->bold_italic, 0);
    }
    
    return 0;
}

/*=============================================================================
 * Utility Functions
 *===========================================================================*/

int stb_font_file_exists(const char* filename) {
    if (!filename) {
        return 0;
    }
    
    char full_path[512];
    build_full_path(filename, full_path, sizeof(full_path));
    
    FILE* fp = fopen(full_path, "rb");
    if (!fp) {
        return 0;
    }
    
    fclose(fp);
    return 1;
}

void stb_font_set_directory(const char* directory) {
    g_font_directory = directory;
}

const char* stb_font_get_directory(void) {
    return g_font_directory;
}
