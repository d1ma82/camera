#ifndef _OPENGL_H
#define _OPENGL_H

#include <stdint.h>

namespace ogl {

    typedef struct {
        int32_t screen_width =0;
        int32_t screen_height = 0;
        int32_t buffer_width = 0;
        int32_t buffer_height = 0;
        int32_t sensor_orient = 0;
        int32_t texture_id = 0;
    } Properties;

    void init_surface (const Properties& properties);
    void next_shader();
    void draw_frame();
    void destroy();
}
#endif