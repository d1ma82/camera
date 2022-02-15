#ifndef _OPENGL_H
#define _OPENGL_H

#include <stdint.h>

namespace ogl {
    void init_surface (int32_t screen_width, int32_t screen_height, int32_t tex_id);
    void next_shader();
    void draw_frame(const float texMat[]);
    void destroy();
}
#endif