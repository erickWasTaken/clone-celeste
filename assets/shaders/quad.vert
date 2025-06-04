void main(){
    vec2 vertices[3] = {
        vec2(0, .5),
        vec2(.5, -.5),
        vec2(-.5, -.5)
    };

    gl_Position = vec4(vertices[gl_VertexID], 1.0, 1.0);
}
