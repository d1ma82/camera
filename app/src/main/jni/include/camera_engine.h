#ifndef _CAMERA_ENGINE_H
#define _CAMERA_ENGINE_H

#include <jni.h>

#include "camera_manager.h"

class CameraEngine { 
private:
    JNIEnv* env;
    jobject instance;
    jstring requested_facing;
    int32_t requested_width;
    int32_t requested_height;
    jobject surface;
    NDKCamera* camera;
    int32_t compatible_res[3] {0,0,0};

public: 
    explicit CameraEngine(JNIEnv* env, jobject instance, jstring facing, jint w, jint h);
    ~CameraEngine();  

    void create_session(jobject surface);
    void start_preview(bool start);
    const jintArray get_compatible_res() const;
    int32_t get_sensor_orientation();
    jobject get_surface();
};

#endif