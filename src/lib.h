#pragma once
#ifdef __linux__
#define DEBUG_BREAK() __builtin_trap()
#define EXPORT_FN
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>

#define ArraySize(x) (sizeof((x)) / sizeof((x)[0]))

#define b8 char
#define BIT(x) 1 << (x)
#define KB(x) ((unsigned long long)1024 * x)
#define MB(x) ((unsigned long long)1024 * KB(x))
#define GB(x) ((unsigned long long)1024 * MB(x))

enum TextColor
{  
  TEXT_COLOR_BLACK,
  TEXT_COLOR_RED,
  TEXT_COLOR_GREEN,
  TEXT_COLOR_YELLOW,
  TEXT_COLOR_BLUE,
  TEXT_COLOR_MAGENTA,
  TEXT_COLOR_CYAN,
  TEXT_COLOR_WHITE,
  TEXT_COLOR_BRIGHT_BLACK,
  TEXT_COLOR_BRIGHT_RED,
  TEXT_COLOR_BRIGHT_GREEN,
  TEXT_COLOR_BRIGHT_YELLOW,
  TEXT_COLOR_BRIGHT_BLUE,
  TEXT_COLOR_BRIGHT_MAGENTA,
  TEXT_COLOR_BRIGHT_CYAN,
  TEXT_COLOR_BRIGHT_WHITE,
  TEXT_COLOR_COUNT
};

template <typename ...Args>
void _log(const char* prefix, const char* msg, TextColor textColor, Args... args){
    static const char* TextColorTable[TEXT_COLOR_COUNT] = {
        "\x1b[30m", // TEXT_COLOR_BLACK
        "\x1b[31m", // TEXT_COLOR_RED
        "\x1b[32m", // TEXT_COLOR_GREEN
        "\x1b[33m", // TEXT_COLOR_YELLOW
        "\x1b[34m", // TEXT_COLOR_BLUE
        "\x1b[35m", // TEXT_COLOR_MAGENTA
        "\x1b[36m", // TEXT_COLOR_CYAN
        "\x1b[37m", // TEXT_COLOR_WHITE
        "\x1b[90m", // TEXT_COLOR_BRIGHT_BLACK
        "\x1b[91m", // TEXT_COLOR_BRIGHT_RED
        "\x1b[92m", // TEXT_COLOR_BRIGHT_GREEN
        "\x1b[93m", // TEXT_COLOR_BRIGHT_YELLOW
        "\x1b[94m", // TEXT_COLOR_BRIGHT_BLUE
        "\x1b[95m", // TEXT_COLOR_BRIGHT_MAGENTA
        "\x1b[96m", // TEXT_COLOR_BRIGHT_CYAN
        "\x1b[97m", // TEXT_COLOR_BRIGHT_WHITE
    };
    char formatBuffer[8192] = {};
    sprintf(formatBuffer, "%s %s %s \033[0m", TextColorTable[textColor], prefix, msg);

    char textBuffer[8192] = {};
    sprintf(textBuffer, formatBuffer, args...);
    puts(textBuffer);
}

#define SM_TRACE(msg, ...) _log("TRACE: ", msg, TEXT_COLOR_GREEN, ##__VA_ARGS__);
#define SM_WARN(msg, ...) _log("WARN: ", msg, TEXT_COLOR_YELLOW, ##__VA_ARGS__);
#define SM_ERROR(msg, ...) _log("ERROR: ", msg, TEXT_COLOR_RED, ##__VA_ARGS__);

#define SM_ASSERT(x, msg, ...)    \
{                                 \
  if(!(x))                        \
  {                               \
    SM_ERROR(msg, ##__VA_ARGS__); \
    DEBUG_BREAK();                \
    SM_ERROR("Assertion HIT!")    \
  }                               \
}

struct BumpAllocator{
    size_t capacity;
    size_t used;
    char* memory;
};

BumpAllocator make_bump_allocator(size_t size){
    BumpAllocator ba = {};
    
    ba.memory = (char*)malloc(size);
    if(!ba.memory){
        SM_ASSERT(false, "Failed to allocate memory!");
    }else{
        ba.capacity = size;
        memset(ba.memory, 0, size);
    }
    return ba; 
}

char* bump_alloc(BumpAllocator* bumpAllocator, size_t size){
    char* result = nullptr;

    size_t allignedSize = (size + 7) & ~ 7; // This ensures that teh first 4 bits are 0
    if(bumpAllocator->used + allignedSize <= bumpAllocator->capacity){
        result = bumpAllocator->memory + bumpAllocator->used;
        bumpAllocator->used += allignedSize;
    }else{
        SM_ASSERT(false, "bumpAllocator is full");
    }

    return result;
}

long get_file_size(const char* filePath){
    SM_ASSERT(filePath, "No file path specified!");

    long fileSize = 0;
    auto file = fopen(filePath, "rb");
    if(!file){
        SM_ERROR("Failed opening file: %s", filePath);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    fclose(file);

    return fileSize;
}

char* read_file(const char* filePath, int* fileSize, char* buffer){
    SM_ASSERT(filePath, "No file path supplied!");
    SM_ASSERT(fileSize, "No file size specified!");
    SM_ASSERT(buffer, "No buffer specified!");

    *fileSize = 0;
    auto file = fopen(filePath, "rb");
    if(!file){
        SM_ERROR("Failed opening file: %s", filePath);
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    memset(buffer, 0, *fileSize + 1);
    fread(buffer, sizeof(char), *fileSize, file);
    
    fclose(file);
    
    return buffer;
}

char* read_file(const char* filePath, int* fileSize, BumpAllocator* bumpAllocator){
    char* file = nullptr;
    long fileSize2 = get_file_size(filePath);
    if(fileSize2){
        char* buffer = bump_alloc(bumpAllocator, fileSize2 + 1);
        file = read_file(filePath, fileSize, buffer);
    }
    
    return file;
}

bool copy_file(const char* fileName, const char* outputName, char* buffer){
    int fileSize = 0;
    char* data = read_file(fileName, &fileSize, buffer);

    auto outputFile = fopen(outputName, "wb");
    if(!outputFile){
        SM_ERROR("Failed opening file: %s", fileName);
        return false;
    }

    int result = fwrite(data, sizeof(char), fileSize, outputFile);
    if(!result){
        SM_ERROR("Failed writing file: %s", fileName);
        return false;
    }

    fclose(outputFile);
    return true;
}

bool copy_file(const char* fileName, const char* outputName, BumpAllocator* bumpAllocator){
    char* file = 0;
    long fileSize2 = get_file_size(fileName);

    if(fileSize2){
        char* buffer = bump_alloc(bumpAllocator, fileSize2 + 1);
        return copy_file(fileName, outputName, buffer);
    }
    return false;
}

long long get_timestamp(const char* file){
    struct stat file_stat = {};
    stat(file, &file_stat);
    return file_stat.st_mtime;
}

template<typename T, int N>
struct Array{
    static constexpr int maxElements = N;
    int count = 0;
    T elements[N];

    T& operator[](int id){
        SM_ASSERT(id >= 0, "negative id requested!");
        SM_ASSERT(id < count, "out of bounds id requested!");
        return elements[id];
    }

    int add(T element){
        SM_ASSERT(count < maxElements, "Array is already full!");
        elements[count] = element;
        return count++;
    }

    void remove_id_and_swap(int id){
        SM_ASSERT(id >= 0, "negative id requested!");
        SM_ASSERT(id < count, "out of bounds id requested!");
        elements[id] = elements[--count];
    }

    void clear(){
        count = 0;
    }

    bool is_full(){
        return count == N;
    }
};

// Math stuff

struct Vec2{
    float x;
    float y;

    Vec2 operator / (float scalar){
        return {x / scalar, y / scalar};
    }

    Vec2 operator * (float scalar){
        return {x * scalar, y * scalar};
    }

    Vec2 operator - (Vec2 vec){
        return {x - vec.x, y - vec.y};
    }

    Vec2 operator + (Vec2 vec){
        return {x + vec.x, x + vec.x};
    }

    float magnitude(){
        return abs(sqrt(x * x + y * y));
    }

    Vec2 unit(){
        return {x / magnitude(), y / magnitude()};
    }

    operator bool(){
        return x != 0.0f && 0.0f;
    }
};

struct IVec2{
    int x;
    int y;

    IVec2 operator - (IVec2 vec){
        return {x - vec.x, y - vec.y};
    }

    IVec2 operator + (IVec2 vec){
        return {x + vec.x, y + vec.y};
    }

    IVec2& operator -= (int scalar){
        x -= scalar;
        y -= scalar;
        return *this;
    }

    IVec2& operator += (int scalar){
        x += scalar;
        y += scalar;
        return *this;
    }

    IVec2 operator / (int scalar){
        return {x / scalar, y / scalar};
    }

    IVec2 operator * (int scalar){
        return {(int)x * scalar, (int)y * scalar};
    }

    float magnitude(){
        return abs(sqrt(x * x + y * y));
    }

    Vec2 unit(){
        return {x / magnitude(), y / magnitude()};
    }
};

struct Vec4{
    union{
        float values[4];
        struct{
            float x;
            float y;
            float z;
            float w;
        };

        struct{
            float r;
            float q;
            float b;
            float a;
        };
    };

    float& operator[](int id){
        return values[id];
    }

    bool operator ==(Vec4 vec){
        return x == vec.x && y == vec.y && z == vec.z && w == vec.w;
    }
};

struct Mat4{
    union{
        Vec4 values[4];
        struct{
            float ax;
            float bx;
            float cx;
            float dx;

            float ay;
            float by;
            float cy;
            float dy;

            float az;
            float bz;
            float cz;
            float dz;

            float aw;
            float bw;
            float cw;
            float dw;
        };
    };

    Vec4& operator[](int col){
        return values[col];
    }
};

Mat4 orthographic_projection(float left, float right, float top, float bottom, float zoom){
    Mat4 result = {};
    result.aw = -(right + left) / (right - left);
    result.bw = (top + bottom) / (top - bottom);
    result.cw = 0.0f;

    result[0][0] = 2.0f / (right - left) * zoom;
    result[1][1] = 2.0f / (top - bottom) * zoom;
    result[2][2] = 1.0f / (1.0f - 0.0f) * zoom; // far and near plane
    result[3][3] = 1.0f;

    return result;
}

Vec2 toVec2(IVec2 vec){
    return Vec2{(float)vec.x, (float)vec.y};
}

int max(int a, int b){
    if(a > b)
        return a;
    return b;
}

float max(float a, float b){
    if(a > b)
        return a;
    return b;
}

long long max(long long a, long long b){
    if(a > b)
        return a;
    return b;
}

float min(float a, float b){
    if(a > b){
        return b;
    }
    return a;
}

float lerp(float a, float b, float t){
    return a + (b - a) * t;
}

int sign(int n){
    return (n >= 0)? 1 : -1;
}

IVec2 lerp(IVec2 a, IVec2 b, float t){
    IVec2 result;
    result.x = lerp(a.x, b.x, t);
    result.y = lerp(a.y, b.y, t);
    return result;
}

float approach(float current, float target, float increase){
    if(current < target){
        return min(current + increase, target);
    }else{
        return max(current - increase, target);
    }
}

Vec2 approarch(Vec2 current, Vec2 target, float increase){
    Vec2 result;

    result.x = approach(current.x, target.x, increase);
    result.y = approach(current.y, target.y, increase);
    return result;
}


float dot(IVec2 a, IVec2 b){
    return a.unit().x * b.unit().x + a.unit().y * b.unit().y;
}

int round_to_int(float n){
    float remainderDown = n - floor(n);
    float remainderUp = ceil(n) - n;

    if(remainderUp > remainderDown)
        return floor(n);
    return ceil(n);
}

struct IRect{
    IVec2 pos;
    IVec2 size;
};

