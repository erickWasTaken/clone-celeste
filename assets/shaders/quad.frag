layout (location = 0) in vec2 texCoordIn;

layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D texAtlas;

void main(){
    vec2 localUV = fract(texCoordIn); // Range: [0.0, 1.0)
    vec2 diff = localUV - vec2(0.5);   // Distance from center of texel
    float dist = length(diff);        // Euclidean distance
    vec4 texColor = texelFetch(texAtlas, ivec2(texCoordIn), 0);
    
    // Dotted alpha falloff based on distance to texel center
    texColor.a *= 1.0 - smoothstep(0.3, 0.5, dist); // Optional: soften edge
    
    if(texColor.a < .3){
        discard;
    } else {
        fragColor = texColor;
    }
}
