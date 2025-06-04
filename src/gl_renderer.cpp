#include "gl_renderer.h"

struct GLContext{
    GLuint programID;
    long long shaderTimestamp;

};

static GLContext glContext;

GLuint gl_create_shader(int shaderType, char* shaderPath, BumpAllocator* transientStorage){
    int fileSize = 0;
    char* shaderHeader = read_file("src/shader_header.h", &fileSize, transientStorage);
    char* shaderSource = read_file(shaderPath, &fileSize, transientStorage);
    if(!shaderHeader){
        SM_ASSERT(false, "Failed to load shader header!");
        return 0;
    }
    if(!shaderSource){
        SM_ASSERT(false, "Failed to load shader: %s", shaderPath);
        return 0;
    }
    
    char* shaderSources[] = {
        "#version 430 core\r\n",
        shaderHeader,
        shaderSource
    };

    GLuint shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, ArraySize(shaderSources), shaderSources, 0);
    glCompileShader(shaderID);

    {
        int success;
        char shaderLog[2048] = {};

        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
        if(!success){
            glGetShaderInfoLog(shaderID, 2048, 0, shaderLog);
            SM_ASSERT(false, "Failed to compile %s shader! ERROR: %s", shaderPath, shaderLog);
            return 0;
        }
    }
    return shaderID;
}

bool gl_init(BumpAllocator* transientStorage){
    load_gl_functions();

    GLuint vertShaderID = gl_create_shader(GL_VERTEX_SHADER, "assets/shaders/quad.vert", transientStorage);
    GLuint fragShaderID = gl_create_shader(GL_FRAGMENT_SHADER, "assets/shaders/quad.frag", transientStorage);

    if(!vertShaderID || !fragShaderID){
        SM_ASSERT(false, "Failed to create shaders");
        return false;
    }

    long long timestampVert = get_timestamp("assets/shaders/quad.vert");
    long long timestampFrag = get_timestamp("assets/shaders/quad.frag");
    
    glContext.shaderTimestamp = max(timestampVert, timestampFrag);
    glContext.programID = glCreateProgram();
    glAttachShader(glContext.programID, vertShaderID);
    glAttachShader(glContext.programID, fragShaderID);
    glLinkProgram(glContext.programID);

    {
        int success;
        char infoLog[512];
        glGetProgramiv(glContext.programID, GL_LINK_STATUS, &success);

        if(!success){
            glGetProgramInfoLog(glContext.programID, 512, 0, infoLog);

            SM_ASSERT(0, "Failed to link program: %s", infoLog);
            return false;
        }
    }

    glDetachShader(glContext.programID, vertShaderID);
    glDetachShader(glContext.programID, fragShaderID);
    glDeleteShader(vertShaderID);
    glDeleteShader(fragShaderID);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glEnable(GL_FRAMEBUFFER_SRGB);
    glDisable(0x809D); // disable multismapling (???)
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);

    glUseProgram(glContext.programID);

    return true;
}

void gl_render(BumpAllocator* transientStorage){
    glClearColor(.0f, .0f, .0f, .0f);
    glClearDepth(.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, 1280, 720);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 1);
}
