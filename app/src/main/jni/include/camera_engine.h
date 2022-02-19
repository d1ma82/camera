#ifndef _CAMERA_ENGINE_H
#define _CAMERA_ENGINE_H

#include "camera_manager.h"
#include <string>
#include <jni.h>

class CameraEngine { 
private:

    JNIEnv* env;
    jobject instance;
    jstring requested_facing;
    NDKCamera* camera;
    std::string dcim;

public: 
    explicit CameraEngine(JNIEnv* env, jobject instance, jstring facing, jstring dcim);
       // copy-move disabled
    CameraEngine(const CameraEngine&) = delete;
    CameraEngine(CameraEngine&&) = delete;
    CameraEngine& operator=(const CameraEngine&) = delete;
    CameraEngine& operator=(CameraEngine&&) = delete;
    ~CameraEngine();  

    NDKCamera* operator->() {return camera;}

    inline const char* get_dcim() const {return dcim.c_str();}
};

#endif