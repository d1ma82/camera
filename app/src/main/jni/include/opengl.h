#ifndef _OPENGL_H
#define _OPENGL_H

#include <stdint.h>

namespace ogl {
    void init_surface (int32_t tex_id);
    void draw_frame(int32_t width, int32_t height, const float texMat[]);
    void surface_changes();
}
#endif