#ifndef _OPENGL_H
#define _OPENGL_H

#include <stdint.h>

void prepare (uint32_t tex_id);
void draw_frame(float texMat[16]);
void surface_changes(int32_t width, int32_t height);

#endif