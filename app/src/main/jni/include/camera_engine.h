#ifndef _CAMERA_ENGINE_H
#define _CAMERA_ENGINE_H

#include "camera_manager.h"

#include <jni.h>

class CameraEngine { 
private:
    JNIEnv* env;
    jobject instance;
    jstring requested_facing;
    NDKCamera* camera;
    int32_t compatible_res[3] {0,0,0};

public: 
    explicit CameraEngine(JNIEnv* env, jobject instance, jstring facing);
    ~CameraEngine();  

    void create_session(jobject surface);
    void init_surface(int32_t texture_id);
    void start_preview(bool start);
    void draw_frame(const jfloatArray texMatArray);
    const jintArray get_compatible_res(jint width, jint height) const;
    int32_t get_sensor_orientation();
};

#endif