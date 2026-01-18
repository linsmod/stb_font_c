# stb_font_cache - C Font Rendering Library

A lightweight C wrapper around [stb_truetype](https://github.com/nothings/stb) for font rendering with SDL support.

## Features

- **Pure C API**: No C++ required, easy to integrate into C projects
- **TrueType Font Support**: Load .ttf and .ttc font files
- **Font Caching**: Efficient glyph caching for performance
- **Text Measurement**: Get width and height of text strings
- **Formatted Text**: Support for colors, bold, italic, underline, strikethrough
- **SDL Rendering**: Built-in SDL2/SDL3 support for easy rendering
- **Prerendered Text**: Cache rendered text to textures for static content
- **Multiple Fonts**: Support for fallback fonts
- **UTF-8 Support**: Full Unicode text support

## Building

### Prerequisites

- C compiler (gcc, clang, msvc, etc.)
- SDL3 (or SDL2 with minor modifications)
- CMake (optional, for building the example)

### Compiling

Using CMake:

```bash
mkdir build && cd build
cmake ..
make
./example
```

Using gcc manually:

```bash
gcc -DSTB_FONT_SDL_ENABLED -I. stb_font_cache.c example.c -o example `sdl3-config --libs`
```

## Quick Start

```c
#include "stb_font_cache.h"

// Create font cache
stb_font_cache_t* cache = stb_font_cache_create();
stb_font_cache_set_face_size(cache, 24);

// Load font
stb_font_memory_t font_mem;
stb_font_memory_alloc(&font_mem, font_data_size);
memcpy(font_mem.data, font_data, font_data_size);
stb_font_load_managed(cache, &font_mem, 0);

// Bind SDL renderer
SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
stb_font_sdl_bind_renderer(cache, renderer);

// Draw text
stb_font_draw_text(cache, 10, 10, "Hello, World!", -1);

// Cleanup
stb_font_cache_destroy(cache);
```

## API Reference

### Core Functions

#### `stb_font_cache_create()`
Creates a new font cache instance.

```c
stb_font_cache_t* stb_font_cache_create(void);
```

#### `stb_font_cache_destroy()`
Destroys a font cache and frees all resources.

```c
void stb_font_cache_destroy(stb_font_cache_t* cache);
```

#### `stb_font_cache_set_face_size()`
Sets the font size in pixels. Must be called before loading a font.

```c
void stb_font_cache_set_face_size(stb_font_cache_t* cache, int face_size);
```

### Font Loading

#### `stb_font_load_managed()`
Loads a font from memory and takes ownership of the memory.

```c
int stb_font_load_managed(stb_font_cache_t* cache, 
                         stb_font_memory_t* memory, 
                         int index);
```

Parameters:
- `cache`: Font cache
- `memory`: Memory structure containing font data (will be freed when cache is destroyed)
- `index`: Font index (for .ttc files), typically 0

Returns: 0 on success, non-zero on failure

#### `stb_font_add_managed()`
Adds a fallback font for characters not in the primary font.

```c
int stb_font_add_managed(stb_font_cache_t* cache, 
                        stb_font_memory_t* memory, 
                        int index);
```

#### `stb_font_add_format_font_managed()`
Adds a format variant (bold, italic, etc.) for the last loaded font.

```c
int stb_font_add_format_font_managed(stb_font_cache_t* cache, 
                                    uint8_t format_mask, 
                                    stb_font_memory_t* memory, 
                                    int index);
```

Format flags:
- `STB_FONT_FORMAT_BOLD`
- `STB_FONT_FORMAT_ITALIC`
- `STB_FONT_FORMAT_UNDERLINE`
- `STB_FONT_FORMAT_STRIKETHROUGH`

### Text Measurement

#### `stb_font_get_text_size()`
Measures the size of a plain text string.

```c
void stb_font_get_text_size(stb_font_cache_t* cache, 
                           const char* text, 
                           int max_len, 
                           int* width_out, 
                           int* height_out);
```

Parameters:
- `cache`: Font cache
- `text`: UTF-8 string to measure
- `max_len`: Maximum length in bytes (use -1 for null-terminated)
- `width_out`: Output width (can be NULL)
- `height_out`: Output height (can be NULL)

### Text Rendering

#### `stb_font_draw_text()`
Draws plain text at specified position.

```c
int stb_font_draw_text(stb_font_cache_t* cache, 
                      int x, int y, 
                      const char* text, 
                      int max_len);
```

Returns: X position after drawing

#### `stb_font_draw_text_formatted()`
Draws formatted text at specified position.

```c
int stb_font_draw_text_formatted(stb_font_cache_t* cache, 
                                int x, int y, 
                                const char* text, 
                                const stb_font_text_format_t* format, 
                                int max_len);
```

### Text Formatting

#### `stb_font_format_color()`
Creates a text format with specified color.

```c
stb_font_text_format_t stb_font_format_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
```

#### `stb_font_format_flags()`
Creates a text format with specified flags.

```c
stb_font_text_format_t stb_font_format_flags(uint8_t flags);
```

#### `stb_font_format_combine()`
Combines two text formats.

```c
void stb_font_format_combine(stb_font_text_format_t* dst, 
                           const stb_font_text_format_t* src);
```

### SDL-Specific Functions

#### `stb_font_sdl_bind_renderer()`
Binds an SDL renderer for rendering.

```c
void stb_font_sdl_bind_renderer(stb_font_cache_t* cache, void* renderer);
```

#### `stb_font_sdl_render_to_object()`
Renders text to a prerendered text object.

```c
void stb_font_sdl_render_to_object(stb_font_cache_t* cache, 
                                   stb_prerendered_text_t* text_out, 
                                   const char* text, 
                                   int max_len);
```

#### `stb_font_sdl_draw_prerendered()`
Draws a prerendered text object.

```c
int stb_font_sdl_draw_prerendered(stb_prerendered_text_t* text, int x, int y);
```

## Example Usage

### Basic Text Rendering

```c
// Create and setup font cache
stb_font_cache_t* cache = stb_font_cache_create();
stb_font_cache_set_face_size(cache, 32);

// Load font
stb_font_memory_t font_mem;
stb_font_memory_alloc(&font_mem, font_size);
memcpy(font_mem.data, font_data, font_size);
stb_font_load_managed(cache, &font_mem, 0);

// Bind renderer
stb_font_sdl_bind_renderer(cache, renderer);

// Draw text
stb_font_draw_text(cache, 50, 50, "Hello, World!", -1);
```

### Formatted Text

```c
// Create red bold format
stb_font_text_format_t red_bold = stb_font_format_color(255, 0, 0, 255);
red_bold.format = STB_FONT_FORMAT_BOLD;

// Draw formatted text
stb_font_draw_text_formatted(cache, 50, 50, "Bold Red Text", &red_bold, -1);
```

### Prerendered Text

```c
// Create prerendered text
stb_prerendered_text_t text;
stb_font_text_format_t fmt = stb_font_format_color(255, 255, 0, 255);
stb_font_sdl_render_formatted_to_object(cache, &text, "Static Text", &fmt, -1);

// Draw repeatedly (no re-rendering needed)
for (int i = 0; i < 100; i++) {
    stb_font_sdl_draw_prerendered(&text, 10, 10 + i * 20);
}

// Cleanup
stb_font_sdl_free_prerendered(&text);
```

### Text Measurement

```c
int width, height;
stb_font_get_text_size(cache, "Measure me!", -1, &width, &height);

printf("Text size: %dx%d\n", width, height);
```

## License

Public Domain / CC0 - Feel free to use however you want

## Credits

- [stb_truetype](https://github.com/nothings/stb) by Sean Barrett - Single-file TrueType font parsing library
- Original inspiration from [sdl-stb-font](https://github.com/liamtwigger/sdl-stb-font) by Liam Twigger

## Notes

- This is a simplified C implementation. Some advanced features from the original C++ version are not yet implemented:
  - HarfBuzz shaping for complex scripts
  - Multi-line text wrapping
  - Kerning optimization
  - Advanced text layout features
  
- The library uses a simple hash-based glyph cache. Performance may degrade with very large character sets.
  
- For production use, consider integrating HarfBuzz for proper text shaping and layout of complex scripts (Arabic, Hebrew, etc.)

## Building Your Own Rendering Backend

The library is designed to be extensible. To create a custom rendering backend:

1. Implement the core `stb_font_*` functions (text measurement, etc.)
2. Implement your own `draw_text_worker()` function
3. Provide platform-specific texture management functions

See `stb_font_sdl_*` functions in `stb_font_cache.c` for a reference implementation.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.
