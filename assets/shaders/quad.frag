layout (location = 0) in vec2 texCoordIn;

layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D texAtlas;

void main(){
    vec4 texColor = texelFetch(texAtlas, ivec2(texCoordIn), 0);
    if(texColor.a == 0.0){
        discard;
    }else
        fragColor = texColor;
}
