#ifndef _CUSTOM_CAMERA_H
#define _CUSTOM_CAMERA_H

#include <stdint.h>

class CustomCamera {
public:
    virtual ~CustomCamera() = default;
    virtual void create_session(void* window) = 0;
    virtual void close_session() = 0;
    virtual void start_preview(bool start) = 0;
    virtual void take_photo(const char* path) = 0;
    virtual void calc_compatible_preview_size(int32_t width, int32_t height, int32_t out_compatible_res[2]) = 0;
    virtual void init_surface(int32_t texture_id) = 0;
    virtual void next_shader() = 0;
    virtual void draw_frame() = 0;
protected:

    CustomCamera(){};
};

#endif