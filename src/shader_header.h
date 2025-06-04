#define BIT(i) 1 << i

struct Transform{
    vec2 pos;
    vec2 size;
    ivec2 atlasOffset;
    ivec2 spriteSize;
    int renderOptions;
    int materialIdx;
    float layer;
    int padding;
};

struct Material{
    vec4 color;
};
