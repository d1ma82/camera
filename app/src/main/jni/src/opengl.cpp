#include "opengl.h"
#include "log.h"
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

GLuint texture_id = 0, vertex_shader, fragment_shader, program;
GLint vertex_position, st, tex_sampler, tex_matrix;
GLuint buffers[2];

static int32_t width = 0, height = 0;

static const char* vertex_shader_src = R"(
    
    attribute vec3 vertex_position;
    attribute vec2 st;
    uniform mat4 texMatrix;
    varying vec2 var_st;
    
    void main()
    {
       // varUvs = (texMatrix * vec4(uv.x, uv.y, 0, 1.0)).xy;
        var_st = st;
        gl_Position = vec4(vertex_position, 1.0);
    }
)";

static const char* fragment_shader_src = R"(
 
    #extension GL_OES_EGL_image_external : require
    precision mediump float;
    
    uniform samplerExternalOES texSampler;
    varying vec2 var_st;
    
    void main()
    {
        gl_FragColor = texture2D(texSampler, var_st);
       //gl_FragColor = vec4(1, 0, 0, 1);  // red
    }    
)";

// Note that the coordinates of this shape are defined in a counterclockwise order. 
// The drawing order is important because it defines which side is the front face of the shape, 
// which you typically want to have drawn, and the back face, which you can choose to not draw 
// using the OpenGL ES cull face feature
static float vertices[] {
//   x   y   z    s   t  
    -1,  1, .0f,  0,  1,      // top left
    -1, -1, .0f,  0,  0,      // bottom left  
     1, -1, .0f,  1,  0,      // bottom right
     1,  1, .0f,  1,  1       // top right  
};

static GLuint indices[] { 0, 1, 2, 0, 2, 3 };


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

void ogl::init_surface (int32_t w, int32_t h, int32_t tex_id) {

    if (texture_id == tex_id) return;

    texture_id = tex_id; width = w; height = h;
    LOGI("Screen size %d x %d; texture_id = %d", width, height, texture_id)
    vertex_shader = create_shader(vertex_shader_src, GL_VERTEX_SHADER);
    fragment_shader = create_shader(fragment_shader_src, GL_FRAGMENT_SHADER);
    program = create_program(vertex_shader, fragment_shader);

    vertex_position = glGetAttribLocation(program, "vertex_position");
    st =              glGetAttribLocation(program, "st");
    tex_sampler =     glGetUniformLocation(program, "texSampler");
    tex_matrix =      glGetUniformLocation(program, "texMatrix");

    glGenBuffers(2, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
}

void ogl::draw_frame(const float texMat[]) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(0,0,0,1);

    glUseProgram(program);

  /*  float ratio = float(width) / float(height);
    // create a projection matrix from device screen geometry
    glm::mat4x4 proj = glm::ortho(-ratio, ratio, -ratio, ratio, 3.0f, 7.0f);
    glm::vec3 eye       {0, 0, 3.0f};
    glm::vec3 center    {0, 0, 0};
    glm::vec3 up        {0, 1.0f, 0};
    glm::mat4x4 view = glm::lookAt(eye, center, up); // camera view
    glm::mat4x4 mvp_data  = proj * view;*/

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1i(tex_sampler, 0);

   // glUniformMatrix4fv(tex_matrix, 1, false, texMat);
  
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glEnableVertexAttribArray(vertex_position);
    glVertexAttribPointer(vertex_position, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
        
    glEnableVertexAttribArray(st);
    glVertexAttribPointer(st, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)(3 * sizeof(float)));
    
    glViewport(0, 0, width, height);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);  // use indices
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(vertex_position);
    glDisableVertexAttribArray(st);
}

void ogl::destroy() {

    glDeleteProgram(program);
    glDeleteBuffers(2, buffers);
}