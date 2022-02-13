#ifndef _OPENGL_H
#define _OPENGL_H

#include <stdint.h>

namespace ogl {
    void init_surface (int32_t width, int32_t height, int32_t aspect, int32_t tex_id);
    void draw_frame(const float texMat[]);
    void destroy();
}
#endif