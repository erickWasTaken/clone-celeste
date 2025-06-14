
layout (location = 0) out vec2 texCoordOut;

layout (std430, binding = 0) buffer TransformSBO{
    Transform transforms[];
};

uniform vec2 screenSize;
uniform mat4 orthoProjection;

void main(){
    Transform transform = transforms[gl_InstanceID];

    vec2 vertices[6] = {
        transform.pos,                                      // top left                      
        vec2(transform.pos + vec2(0.0, transform.size.y)),  // bottom left
        vec2(transform.pos + vec2(transform.size.x, 0.0)),  // top right

        vec2(transform.pos + vec2(transform.size.x, 0.0)),  // top right
        vec2(transform.pos + vec2(0.0, transform.size.y)),  // bottom left
        transform.pos + transform.size                     // bottom right
    };

    // vec2 vertices[6] = {
    //     vec2(-0.5, 0.5),
    //     vec2(-0.5, -0.5),
    //     vec2(0.5, 0.5),
    //
    //     vec2(0.5, 0.5),
    //     vec2(-0.5, -0.5),
    //     vec2(0.5, -0.5)
    // };

    float left = transform.atlasOffset.x;
    float top = transform.atlasOffset.y;
    float right = transform.atlasOffset.x + transform.spriteSize.x;
    float bottom = transform.atlasOffset.y + transform.spriteSize.y;

    vec2 textureCoords[6] = {
        vec2(left, top),
        vec2(left, bottom),
        vec2(right, top),

        vec2(right, top),
        vec2(left, bottom),
        vec2(right, bottom)
    };

    vec2 vertexPos = vertices[gl_VertexID];
    // vertexPos.y = -vertexPos.y + screenSize.y;
    // vertexPos = 2.0 * (vertexPos / screenSize) - 1.0;
    gl_Position = orthoProjection * vec4(vertexPos, 0.0, 1.0);
    
    texCoordOut = textureCoords[gl_VertexID];
}

