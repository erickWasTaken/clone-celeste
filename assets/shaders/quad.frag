layout (location = 0) in vec2 texCoordIn;
layout (location = 1) in flat int renderOptions;

layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D texAtlas;
layout (binding = 1) uniform sampler2D fontAtlas;

void main(){
    vec4 texColor = vec4(.0f);
    if(bool(renderOptions & FONT)){
        texColor = texelFetch(fontAtlas, ivec2(texCoordIn), 0);
        if(texColor.r == 0.0)
            discard;

        fragColor = vec4(1.0f);
        return;
    }
    else{
        texColor = texelFetch(texAtlas, ivec2(texCoordIn), 0);
    
        if(texColor.a == 0.0){
            discard;
        } else {
            fragColor = texColor;
        }
    }
}
