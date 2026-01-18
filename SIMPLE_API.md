# stb_font_simple - Simplified Font API

这是一个 `stb_font_cache` 的简化wrapper，旨在减少样板代码，使字体加载和使用更加简单直观。

## 主要优势

### 原始API vs 简化API

**原始API (复杂)**:
```c
stb_font_memory_t memory = {0};
FILE* fp = fopen("font.ttf", "rb");
fseek(fp, 0, SEEK_END);
long size = ftell(fp);
fseek(fp, 0, SEEK_SET);
stb_font_memory_alloc(&memory, size);
fread(memory.data, 1, size, fp);
fclose(fp);
stb_font_load_managed(cache, &memory, 0);
```
**需要约15行代码**

**简化API (简单)**:
```c
stb_font_load_file(cache, "font.ttf", 0);
```
**仅需1行代码！**

---

## 快速开始

### 1. 最简单的初始化

一行代码完成所有初始化：

```c
#include "stb_font_simple.h"

stb_font_cache_t* cache = stb_font_cache_init_simple(
    renderer_funcs, renderer,  // 渲染器配置
    24,                         // 字体大小
    "fonts/NotoSans-Regular.ttf"  // 字体文件
);

// 使用cache渲染文本...
stb_font_draw_text_formatted(cache, x, y, "Hello, World!", &format, -1);

// 清理
stb_font_cache_destroy(cache);
```

### 2. 设置字体目录

避免重复输入路径前缀：

```c
stb_font_set_directory("fonts");

// 现在可以直接使用文件名
stb_font_cache_t* cache = stb_font_cache_init_simple(
    renderer_funcs, renderer, 24, "NotoSans-Regular.ttf"
);
```

### 3. 批量加载多个字体

使用数组一次加载所有字体：

```c
stb_font_config_t fonts[] = {
    {"NotoSans-Regular.ttf", 0, 0},                     // 主字体
    {"NotoSans-Bold.ttf", STB_FONT_FORMAT_BOLD, 0},     // 粗体
    {"NotoSans-Italic.ttf", STB_FONT_FORMAT_ITALIC, 0}, // 斜体
    {"NotoSansCJKjp-Regular.otf", 0, 0},               // CJK备用字体
    {"NotoSansArabic-Regular.ttf", 0, 0},              // 阿拉伯文备用字体
};

stb_font_cache_t* cache = stb_font_cache_init_multiple(
    renderer_funcs, renderer, 24, fonts, 5
);
```

### 4. 加载字体家族

一次性加载一个字体的所有变体（Regular、Bold、Italic、Bold+Italic）：

```c
// 定义字体家族
stb_font_family_t latin_family = {
    .regular = "NotoSans-Regular.ttf",
    .bold = "NotoSans-Bold.ttf",
    .italic = "NotoSans-Italic.ttf",
    .bold_italic = "NotoSans-BoldItalic.ttf"
};

// 加载为主字体
stb_font_cache_t* cache = stb_font_cache_init_simple(
    renderer_funcs, renderer, 24, "NotoSans-Regular.ttf"
);
stb_font_load_family(cache, &latin_family);

// 添加其他脚本的字体家族作为备用
stb_font_family_t cjk_family = {
    .regular = "NotoSansCJKjp-Regular.otf"
    // 其他变体可以是NULL
};
stb_font_add_family(cache, &cjk_family);
```

### 5. 添加单个备用字体

```c
stb_font_cache_t* cache = stb_font_cache_init_simple(
    renderer_funcs, renderer, 24, "NotoSans-Regular.ttf"
);

// 添加备用字体
stb_font_add_file(cache, "NotoSansCJKjp-Regular.otf", 0);
stb_font_add_file(cache, "NotoSansArabic-Regular.ttf", 0);

// 添加格式变体
stb_font_add_format_file(cache, STB_FONT_FORMAT_BOLD, "NotoSans-Bold.ttf", 0);
```

---

## API 参考

### 初始化函数

#### `stb_font_cache_init_simple()`
创建并初始化字体缓存（最简单的方法）

```c
stb_font_cache_t* stb_font_cache_init_simple(
    const texture_renderer_ops_t* renderer_funcs,
    void* renderer_context,
    int font_size,
    const char* main_font
);
```

#### `stb_font_cache_init_multiple()`
创建并初始化字体缓存，批量加载多个字体

```c
stb_font_cache_t* stb_font_cache_init_multiple(
    const texture_renderer_ops_t* renderer_funcs,
    void* renderer_context,
    int font_size,
    const stb_font_config_t* configs,
    int count
);
```

### 字体加载函数

#### `stb_font_load_file()`
从文件加载主字体

```c
int stb_font_load_file(
    stb_font_cache_t* cache,
    const char* filename,
    int index
);
```

#### `stb_font_add_file()`
添加备用字体

```c
int stb_font_add_file(
    stb_font_cache_t* cache,
    const char* filename,
    int index
);
```

#### `stb_font_add_format_file()`
添加格式变体（粗体、斜体等）

```c
int stb_font_add_format_file(
    stb_font_cache_t* cache,
    uint8_t format_mask,  // STB_FONT_FORMAT_BOLD, STB_FONT_FORMAT_ITALIC, etc.
    const char* filename,
    int index
);
```

#### `stb_font_load_multiple()`
批量加载多个字体

```c
int stb_font_load_multiple(
    stb_font_cache_t* cache,
    const stb_font_config_t* configs,
    int count
);
```

### 字体家族函数

#### `stb_font_load_family()`
加载完整的字体家族

```c
int stb_font_load_family(
    stb_font_cache_t* cache,
    const stb_font_family_t* family
);
```

#### `stb_font_add_family()`
添加字体家族作为备用

```c
int stb_font_add_family(
    stb_font_cache_t* cache,
    const stb_font_family_t* family
);
```

### 工具函数

#### `stb_font_set_directory()`
设置默认字体目录

```c
void stb_font_set_directory(const char* directory);
```

#### `stb_font_get_directory()`
获取当前默认字体目录

```c
const char* stb_font_get_directory(void);
```

#### `stb_font_file_exists()`
检查文件是否存在

```c
int stb_font_file_exists(const char* filename);
```

---

## 数据结构

### `stb_font_config_t`
字体加载配置

```c
typedef struct {
    const char* filename;      // 字体文件路径
    uint8_t format_mask;       // 格式标志（0表示备用字体）
    int index;                 // TTC文件的字体索引
} stb_font_config_t;
```

### `stb_font_family_t`
字体家族配置

```c
typedef struct {
    const char* regular;        // 常规字体（必须）
    const char* bold;           // 粗体（可选）
    const char* italic;         // 斜体（可选）
    const char* bold_italic;    // 粗斜体（可选）
} stb_font_family_t;
```

---

## 完整示例

```c
#include "stb_font_simple.h"
#include <SDL2/SDL.h>

int main(void) {
    // 初始化SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Renderer* renderer = /* ... */;
    
    // 获取渲染器函数
    const texture_renderer_ops_t* funcs = stb_font_create_renderer_funcs();
    
    // 设置字体目录
    stb_font_set_directory("fonts");
    
    // 一行代码初始化字体缓存
    stb_font_cache_t* cache = stb_font_cache_init_simple(funcs, renderer, 24, "NotoSans-Regular.ttf");
    
    // 添加备用字体
    stb_font_add_file(cache, "NotoSansCJKjp-Regular.otf", 0);
    stb_font_add_format_file(cache, STB_FONT_FORMAT_BOLD, "NotoSans-Bold.ttf", 0);
    
    // 渲染文本
    stb_font_text_format_t white = stb_font_format_color(255, 255, 255, 255);
    stb_font_draw_text_formatted(cache, 10, 10, "Hello, 世界!", &white, -1);
    
    stb_font_text_format_t bold = stb_font_format_color(255, 200, 100, 255);
    bold.format = STB_FONT_FORMAT_BOLD;
    stb_font_draw_text_formatted(cache, 10, 50, "粗体中文", &bold, -1);
    
    // 清理
    stb_font_cache_destroy(cache);
    SDL_Quit();
    
    return 0;
}
```

---

## 编译

使用提供的Makefile：

```bash
# 编译simple_example
make simple_example

# 运行
./simple_example
```

或者手动编译：

```bash
gcc -o simple_example simple_example.c stb_font_cache.c stb_font_simple.c sdl_render.c \
    -I. -std=c11 -O2 -DSTB_FONT_SDL_ENABLED -DSTB_FONT_TEXTURE_RENDERER_ENABLED \
    $(sdl2-config --cflags --libs) -lm
```

---

## 设计原则

1. **简化常用操作** - 最常见的情况（从文件加载字体）只需要1行代码
2. **保持灵活性** - 底层API仍然可用，特殊情况可以使用原始API
3. **自动内存管理** - 无需手动分配和释放字体内存
4. **零依赖** - 只依赖stb_font_cache，不引入额外的依赖
5. **向后兼容** - 完全兼容现有的stb_font_cache API

---

## 比较

### 原始API的优点
- 完全控制内存管理
- 支持从内存缓冲区加载
- 更细粒度的控制

### 简化API的优点
- 代码量减少80-90%
- 自动内存管理
- 更少的错误机会
- 更易读和易维护

对于大多数用例，简化API是更好的选择。只有在需要特殊内存管理或从非文件来源加载字体时，才需要使用原始API。
