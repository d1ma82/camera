#include "opengl.h"
#include "log.h"
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <vector>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

GLuint texture_id = 0;
GLint vertex_position, st, tex_sampler, tex_matrix;
GLuint buffers[2];

static int32_t screen_width = 0, screen_height = 0;
static int32_t img_width = 0, img_height = 0;

enum Type {NORMAL, BLUR, COUNT} ;

typedef struct {
    Type type;
    GLuint id, vertex_shader, fragment_shader;
} Program;

std::vector<Program> programs;

Program* current_program = nullptr;

static const char* vertex_shader_src = R"(
    
    attribute vec3 vertex_position;
    attribute vec2 st;
    varying vec2 var_st;
    uniform mat4 tex_matrix; // transform matrix for 90deg camera rotation
    
    void main()
    {
        var_st = (tex_matrix * vec4(st.x, st.y, 0, 1.0)).xy;
        //var_st = st;

        gl_Position = vec4(vertex_position, 1.0);
    }
)";

static const char* fragment_normal_src = R"(
 
    #extension GL_OES_EGL_image_external : require
    precision mediump float;
    
    uniform samplerExternalOES tex_sampler;
    varying vec2 var_st;
    
    void main()
    {
        gl_FragColor = texture2D(tex_sampler, var_st);
       //gl_FragColor = vec4(1, 0, 0, 1);  // red
    }    
)";

// Blur Filter
static const char fragment_blur_src[] = R"(
    
    #extension GL_OES_EGL_image_external : require
    precision highp float; 
    uniform samplerExternalOES tex_sampler;
    varying vec2 var_st;
 
    void main() {
        vec4 sample0, sample1, sample2, sample3;
        float blurStep = 0.5;
        float step = blurStep / 100.0;
        sample0 = texture2D(tex_sampler, var_st - step);
        sample1 = texture2D(tex_sampler, var_st + step);
        sample2 = texture2D(tex_sampler, vec2(var_st.x + step, var_st.y - step));
        sample3 = texture2D(tex_sampler, vec2(var_st.x - step, var_st.y + step));
        gl_FragColor = (sample0 + sample1 + sample2 + sample3) / 4.0;
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



GLuint create_shader (const char* src, GLenum type) {

    GLuint shader = glCreateShader (type);
    glShaderSource (shader, 1, &src, NULL);
    glCompileShader (shader);
    GLint is_compiled=0;
    glGetShaderiv (shader, GL_COMPILE_STATUS, &is_compiled);
    if (is_compiled == GL_FALSE) {

        GLint length=0;
        glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<GLchar> log(length);
        glGetShaderInfoLog(shader, length, &length, log.data());
        LOGI("Could not compile shader %s - %s", src, log.data());
    }
    return shader;
}

void delete_program (Program& program) {

    glDetachShader(program.id, program.vertex_shader);
    glDetachShader(program.id, program.fragment_shader);
    glDeleteShader(program.vertex_shader);
    glDeleteShader(program.fragment_shader);
    glDeleteProgram(program.id);
}

GLuint create_program (GLuint vertex_shader, GLuint fragment_shader) {

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vertex_shader);
    glAttachShader(prog, fragment_shader);
    glLinkProgram(prog);
    GLint is_linked = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &is_linked);
    if (is_linked == GL_FALSE) LOGI("Could not link program");
    return prog;
}

void ogl::next_shader() {

    switch (current_program->type) {
        default: 
        case NORMAL: current_program = &programs[BLUR]; break;
        case BLUR:   current_program = &programs[NORMAL]; break;
    }
}

void ogl::init_surface (int32_t sw, int32_t sh, int32_t iw, int32_t ih, int32_t tex_id) {

    if (texture_id == tex_id) return; // on android side onSurfaceChanged may call couple times

    texture_id = tex_id; screen_width = sw; screen_height = sh;
    img_width = iw; img_height = ih;

    programs.resize(COUNT);

    int i = 0;
    for (auto &el : programs) {

        el.type = Type(i);
        el.vertex_shader = create_shader(vertex_shader_src, GL_VERTEX_SHADER);
        if (i == BLUR) el.fragment_shader = create_shader(fragment_blur_src, GL_FRAGMENT_SHADER); 
        else el.fragment_shader = create_shader(fragment_normal_src, GL_FRAGMENT_SHADER);
        el.id =  create_program(el.vertex_shader, el.fragment_shader);
        i++;
    }
    current_program = &programs[NORMAL];
    vertex_position = glGetAttribLocation(current_program->id, "vertex_position");
    st =              glGetAttribLocation(current_program->id, "st");
    tex_sampler =     glGetUniformLocation(current_program->id, "tex_sampler");
    tex_matrix =      glGetUniformLocation(current_program->id, "tex_matrix");

    glGenBuffers(2, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    LOGI("Init surface %d x %d; texture_id = %d", screen_width, screen_height, texture_id)
}

float aspect_ratio_correction(bool fillScreen,
                              size_t backingWidth,
                              size_t backingHeight,
                              size_t width,
                              size_t height) {
    float backingAspectRatio = (float) backingWidth / (float) backingHeight;
    float targetAspectRatio = (float) width / (float) height;
    float scalingFactor = 1.0f;

    if (fillScreen) {
        if (backingAspectRatio > targetAspectRatio) {
            scalingFactor = (float) backingWidth / (float) width;
        } else {
            scalingFactor = (float) backingHeight / (float) height;
        }
    }

    return scalingFactor;
}

void ogl::draw_frame(const float texMat[]) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(0,0,0,1);

    glUseProgram(current_program->id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1i(tex_sampler, 0);

    glUniformMatrix4fv(tex_matrix, 1, false, texMat);
  
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glEnableVertexAttribArray(vertex_position);
    glVertexAttribPointer(vertex_position, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
        
    glEnableVertexAttribArray(st);
    glVertexAttribPointer(st, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)(3 * sizeof(float)));
    
    glViewport(0, 0, screen_width, screen_height);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);  // use indices
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(vertex_position);
    glDisableVertexAttribArray(st);
}

void ogl::destroy() {

    glUseProgram(0);

    for (auto &el: programs) delete_program(el);

    glDeleteBuffers(2, buffers);
    programs.resize(0);
    current_program = nullptr;
    texture_id = 0;
}