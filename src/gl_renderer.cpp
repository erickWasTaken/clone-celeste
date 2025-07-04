#include "gl_renderer.h"
#include "render_interface.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <ft2build.h>
#include FT_FREETYPE_H

struct GLContext{
    GLuint programID;
    GLuint textureID;
    GLuint fontAtlasID;

    GLuint transformSBOID;
    GLuint screenSizeID;
    GLuint orthoProjectionID;

    long long shaderTimestamp;
    long long textureTimestamp;
};

static GLContext glContext;
const char* TEXTURE_PATH = "assets/textures/Atlas.png";

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

void load_font(char* fontPath, int fontSize){
    FT_Library ftLibrary;
    FT_Init_FreeType(&ftLibrary);


    FT_Face fontFace;
    FT_New_Face(ftLibrary, fontPath, 0, &fontFace);
    FT_Set_Pixel_Sizes(fontFace, 0, fontSize);


    int padding = 2;
    int row = 0;
    int col = padding;


    const int textureWidth = 512;
    char textureBuffer[textureWidth * textureWidth];

    for(FT_ULong i = 32; i < 127; i++){
        FT_UInt glyphID = FT_Get_Char_Index(fontFace, i);
        FT_Load_Glyph(fontFace, glyphID, FT_LOAD_DEFAULT);
        FT_Error error = FT_Render_Glyph(fontFace->glyph, FT_RENDER_MODE_NORMAL);

        if(col + fontFace->glyph->bitmap.width >= textureWidth){
            row += fontSize;
            col = padding;
        }

        renderData->fontHeight = max((fontFace->size->metrics.ascender - fontFace->size->metrics.descender) >> 6, renderData->fontHeight);

        for(unsigned int y = 0; y < fontFace->glyph->bitmap.rows; ++y){
            for(unsigned int x = 0; x < fontFace->glyph->bitmap.width; ++x){
                textureBuffer[(row + y) * textureWidth + col + x] = fontFace->glyph->bitmap.buffer[y * fontFace->glyph->bitmap.width + x];
            }
        }

        Glyph* glyph = &renderData->glyphs[i];
        glyph->textureCoords = {col, row};

        glyph->size = {
            (int)fontFace->glyph->bitmap.width,
            (int)fontFace->glyph->bitmap.rows
        };

        glyph->advance = {
            (float)(fontFace->glyph->advance.x >> 6),
            (float)(fontFace->glyph->advance.y >> 6)
        };

        col += fontFace->glyph->bitmap.width + padding;
    }

    FT_Done_Face(fontFace);
    FT_Done_FreeType(ftLibrary);


    glGenTextures(1, (GLuint*)&glContext.fontAtlasID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, glContext.fontAtlasID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, textureWidth, textureWidth, 0, GL_RED, GL_UNSIGNED_BYTE, (char*)textureBuffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

    // Texture loading
    {
        int width, height, channels;
        char* data = (char*)stbi_load(TEXTURE_PATH, &width, &height, &channels, 4);
        if(!data){
            SM_ASSERT(false, "Failed to load texture: %s", TEXTURE_PATH);
            return false;
        }

        glGenTextures(1, &glContext.textureID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glContext.textureID);

        // set the currently bound texture wrapping/filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        // This setting only matters when using the GLSL texture() function
        // When you use texelFetch() this setting has no effect,
        // because texelFetch is designed for this purpose
        // See: https://interactiveimmersive.io/blog/glsl/glsl-data-tricks/
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glContext.textureTimestamp = get_timestamp(TEXTURE_PATH);
        stbi_image_free(data);
    }

    {
        load_font("assets/fonts/Gridular-Font/Gridular-Regular.ttf", 16);
        // load_font("assets/fonts/AtariClassic-gry3.ttf", 8);
    }

    { // Transform storage buffer
        glGenBuffers(1, &glContext.transformSBOID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.transformSBOID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Transform) * renderData->transforms.maxElements, renderData->transforms.elements, GL_DYNAMIC_DRAW);
    }

    { // Uniforms
        glContext.screenSizeID = glGetUniformLocation(glContext.programID, "screenSize");
        glContext.orthoProjectionID = glGetUniformLocation(glContext.programID, "orthoProjection");
    }

    glEnable(GL_FRAMEBUFFER_SRGB);
    glDisable(0x809D); // disable multismapling (???)
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);

    glUseProgram(glContext.programID);

    return true;
}

void gl_render(BumpAllocator* transientStorage){
    // Texture hot reload
    {
        long long currentTimestamp = get_timestamp(TEXTURE_PATH);

        if(currentTimestamp > glContext.textureTimestamp){
            glActiveTexture(GL_TEXTURE0);
            int width, height, channels;
            char* data = (char*)stbi_load(TEXTURE_PATH, &width, &height, &channels, 4);
            if(data){
                glContext.textureTimestamp = currentTimestamp;
                glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
        }
    }

    // Shader hot reload
    {
        long long timestampVert = get_timestamp("assets/shaders/quad.vert");
        long long timestampFrag = get_timestamp("assets/shaders/quad.frag");

        if(timestampVert > glContext.shaderTimestamp || timestampFrag > glContext.shaderTimestamp){
            GLuint vertShaderID = gl_create_shader(GL_VERTEX_SHADER, "assets/shaders/quad.vert", transientStorage);
            GLuint fragShaderID = gl_create_shader(GL_FRAGMENT_SHADER, "assets/shaders/quad.frag", transientStorage);

            if(!vertShaderID || !fragShaderID){
                SM_ASSERT(false, "failed to hot reload shaders");
                return;
            }

            GLuint programID = glCreateProgram();
            glAttachShader(programID, vertShaderID);
            glAttachShader(programID, fragShaderID);
            glLinkProgram(programID);

            glDetachShader(programID, vertShaderID);
            glDetachShader(programID, fragShaderID);
            glDeleteShader(vertShaderID);
            glDeleteShader(fragShaderID);

            {
                int programSuccess;
                char programInfoLog[512];
                glGetProgramiv(programID, GL_LINK_STATUS, &programSuccess);

                if(!programSuccess){
                    glGetProgramInfoLog(programID, 512, 0, programInfoLog);
                    SM_ASSERT(0, "Failed to link program: %s", programInfoLog);
                    return;
                }
            }

            glDeleteProgram(glContext.programID);
            glContext.programID = programID;
            glUseProgram(programID);

            glContext.shaderTimestamp = max(timestampVert, timestampFrag);
        }
    }

    glClearColor(119.0f / 255.0f, 33.0f / 255.0f, 111.0f / 255.0f, 1.0f);
    glClearDepth(.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, 1280, 720);

    {
        Vec2 screenSize = {(float)1280, (float)720};
        glUniform2fv(glContext.screenSizeID, 1, &screenSize.x);
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.transformSBOID);

    // Game pass
    {
        OrthographicCamera cam = renderData->camera;
        Mat4 orthoProjection = orthographic_projection(
            cam.pos.x - cam.size.x / 2.0f,
            cam.pos.x + cam.size.x / 2.0f,
            cam.pos.y - cam.size.y / 2.0f,
            cam.pos.y + cam.size.y / 2.0f,
            cam.zoom
        );

        glUniformMatrix4fv(glContext.orthoProjectionID, 1, GL_FALSE, &orthoProjection.ax);

        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Transform) * renderData->transforms.count, renderData->transforms.elements);    
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, renderData->transforms.count);
        renderData->transforms.count = 0;
    }

    // UI pass
    {
        OrthographicCamera cam = renderData->camera;
        Mat4 projection = orthographic_projection(
            cam.pos.x - cam.size.x / 2.0f,
            cam.pos.x + cam.size.x / 2.0f,
            cam.pos.y - cam.size.y / 2.0f,
            cam.pos.y + cam.size.y / 2.0f,
            cam.zoom
        );

        glUniformMatrix4fv(glContext.orthoProjectionID, 1, GL_FALSE, &projection.ax);

        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Transform) * renderData->uiTransforms.count, renderData->uiTransforms.elements);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, renderData->uiTransforms.count);
        renderData->uiTransforms.count = 0;
    }

}

