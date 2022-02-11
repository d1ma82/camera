#include "opengl.h"
#include "log.h"
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <vector>

GLuint texture_id, vertex_shader, fragment_shader, program;
GLint vertex_position, uvs, tex_sampler;
GLuint buffers[2];

static const char* vertex_shader_src = R"(
    
    attribute vec4 vertexPosition;
    attribute vec2 uvs;
    varying vec2 varUvs;
    
    void main()
    {
        varUvs = uvs;
        gl_Position = vertexPosition;
    }
)";

static const char* fragment_shader_src = R"(
 
    #extension GL_OES_EGL_image_external : require
    precision mediump float;
    
    uniform samplerExternalOES texSampler;
    varying vec2 varUvs;
    
    void main()
    {
        gl_FragColor = texture2D(texSampler, varUvs);
    }    
)";

float vertices[] {
            // x, y, z, u, v
            -1, -1, 0, 0, 0,
            -1, 1, 0, 0, 1,
            1, 1, 0, 1, 1,
            1, -1, 0, 1, 0
    };

float texture[] {
        //  u v
            0,0,
            0,1,
            1,1,
            1,0
};

GLuint indices[] { 2, 1, 0, 0, 3, 2 };

GLuint create_shader(const char* src, GLenum type) {

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint is_compiled=0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
    if (is_compiled == GL_FALSE) {

        GLint length=0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<GLchar> log(length);
        glGetShaderInfoLog(shader, length, &length, log.data());
        LOGI("Could not compile shader %s - %s", src, log.data());
    }
    return shader;
}

GLuint create_program(GLuint vertex_shader, GLuint fragment_shader) {

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vertex_shader);
    glAttachShader(prog, fragment_shader);
    glLinkProgram(prog);
    GLint is_linked = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &is_linked);
    if (is_linked == GL_FALSE) LOGI("Could not link program");
    return prog;
}

void ogl::init_surface (int32_t tex_id) {

    texture_id = tex_id;
    vertex_shader = create_shader(vertex_shader_src, GL_VERTEX_SHADER);
    fragment_shader = create_shader(fragment_shader_src, GL_FRAGMENT_SHADER);
    program = create_program(vertex_shader, fragment_shader);

    vertex_position = glGetAttribLocation(program, "vertexPosition");
    uvs =             glGetAttribLocation(program, "uvs");
    tex_sampler =     glGetUniformLocation(program, "texSampler");

    glGenBuffers(2, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
}

void ogl::draw_frame(int32_t width, int32_t height, const float texMat[]) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(0,0,0,1);

    glUseProgram(program);

    glEnableVertexAttribArray(vertex_position);
    glVertexAttribPointer(vertex_position, 3, GL_FLOAT, GL_FALSE, 0, vertices);

    glEnableVertexAttribArray(uvs);
    glVertexAttribPointer(uvs, 2, GL_FLOAT, GL_FALSE, 0, texture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUniform1i(tex_sampler, 0);

    glViewport(0, 0, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);
}

void ogl::surface_changes() {

}