#include "opengl.h"
#include "log.h"
#include "matrix.h"
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <vector>

GLint vertex_position, st, tex_sampler, transform;
GLuint buffers[2];

static ogl::Properties properties;
static bool initialized = false;

enum Type {NORMAL, BLUR, PREDATOR, COUNT} ;

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
    uniform mat4 transform;     // transform traditional texture coordinates, 
                                // to the proper sampling location in the streamed texture.
    void main()
    {
        var_st = (transform * vec4(st.x, st.y, 0, 1.0)).xy;
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
    })";

// Predator Thermal Vision Filter
static const char fragment_predator_src[] = R"(

    #extension GL_OES_EGL_image_external : require
    precision highp float;
    uniform samplerExternalOES tex_sampler;
    varying vec2 var_st;

    void main() {
        vec4 color = texture2D(tex_sampler, var_st);
        vec3 colors[3];
        colors[0] = vec3(0.,0.,1.);
        colors[1] = vec3(1.,1.,0.);
        colors[2] = vec3(1.,0.,0.);
        float lum = (color.r + color.g + color.b) /3.;
        int idx = (lum < 0.5) ? 0 : 1;
        vec3 rgb = mix(colors[idx], colors[idx+1], (lum-float(idx) * 0.5) / 0.5);
        gl_FragColor = vec4(rgb, 1.0);
    })";


static float vertices[] {
//   x   y   z    s   t  
    -1,  1, .0f,  0,  1,      // top left
     1, -1, .0f,  1,  0,      // bottom right 
    -1, -1, .0f,  0,  0,      // bottom left 
     1,  1, .0f,  1,  1       // top right  
};

static GLuint indices[] { 0, 1, 2, 0, 1, 3 };

static mtx::mat<float, 4, 4> ident {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1  
};

static mtx::mat<float, 4, 4> st_coord {
        0, 1, 0, 0,
        1, 0, 0, 0,
        0, 0, 0, 0,
        1, 1, 0, 0  
};

static mtx::mat<float, 4, 4> flip_v {
        1,  0, 0, 0,
        0, -1, 0, 0,
        0,  0, 1, 0,
        0,  1, 0, 1  
};

static mtx::mat<float, 4, 4> rot180_z {
       -1,  0, 0, 0,
        0, -1, 0, 0,
        0,  0, 1, 0,
        0,  0, 0, 1,
};

mtx::mat<float, 4, 4> compensate_matrix {ident};

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

void ogl::next_filter() {

    switch (current_program->type) {
        default: 
        case NORMAL:       current_program = &programs[BLUR]; break;
        case BLUR:         current_program = &programs[PREDATOR]; break;
        case PREDATOR:     current_program = &programs[NORMAL]; break;
    }
}

// This code is need to be tested in other devices
void calc_compesate_matrix() {
    
    compensate_matrix = rot180_z * st_coord;
    
    if (properties.sensor_orient == 270) compensate_matrix = flip_v * compensate_matrix;

    LOGI("%s", to_string(compensate_matrix));
}

void ogl::init_surface (const Properties& props) {

    properties = props;             // properties can change during application run, so it need to be updated
    calc_compesate_matrix();        // And recalculate compensate matrix
    if (initialized) return; 

    programs.resize(COUNT);

    int i = 0;
    for (auto &el : programs) {

        el.type = Type(i);
        el.vertex_shader = create_shader(vertex_shader_src, GL_VERTEX_SHADER);

        if (i == BLUR)          el.fragment_shader = create_shader(fragment_blur_src, GL_FRAGMENT_SHADER); 
        else if (i == PREDATOR) el.fragment_shader = create_shader(fragment_predator_src, GL_FRAGMENT_SHADER); 
        else                    el.fragment_shader = create_shader(fragment_normal_src, GL_FRAGMENT_SHADER);

        el.id =  create_program(el.vertex_shader, el.fragment_shader);
        i++;
    }
    current_program = &programs[NORMAL];
    vertex_position = glGetAttribLocation(current_program->id, "vertex_position");
    st =              glGetAttribLocation(current_program->id, "st");
    tex_sampler =     glGetUniformLocation(current_program->id, "tex_sampler");
    transform =       glGetUniformLocation(current_program->id, "transform");

    glClearColor(0,0,0,1);

    glGenBuffers(2, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
    initialized = true;

    LOGI("Init surface %d x %d; texture_id = %d", properties.screen_width, properties.screen_height, properties.texture_id)
}


void ogl::draw_frame() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(current_program->id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, properties.texture_id);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1i(tex_sampler, 0);

    glUniformMatrix4fv(transform, 1, false, &compensate_matrix[0]);
  
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glEnableVertexAttribArray(vertex_position);
    glVertexAttribPointer(vertex_position, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
        
    glEnableVertexAttribArray(st);
    glVertexAttribPointer(st, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)(3 * sizeof(float)));
    
    glViewport(0, 0, properties.screen_width, properties.screen_height);
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
    properties = {0,0,0,0,0,0};
    initialized = false;
}