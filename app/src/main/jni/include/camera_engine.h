#ifndef _CAMERA_ENGINE_H
#define _CAMERA_ENGINE_H

#include "custom_camera.h"
#include <jni.h>

class CameraEngine { 
public: 
    explicit CameraEngine(JNIEnv* env, jobject instance, jstring dcim);
       // copy-move disabled
    CameraEngine(const CameraEngine&) = delete;
    CameraEngine(CameraEngine&&) = delete;
    CameraEngine& operator=(const CameraEngine&) = delete;
    CameraEngine& operator=(CameraEngine&&) = delete;
    ~CameraEngine();  

    CustomCamera* operator->() {return selected;};

    inline const char* get_dcim() const {return dcim;};
    void add_camera_kindof(jstring kind);
    void select_camera(uint32_t id);
    
private:
    JNIEnv* env;
    jobject instance;
    char* dcim = nullptr;
    CustomCamera* selected = nullptr;
};

#endif