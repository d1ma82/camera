/*
 *  2022 NDK Camera2 API project
 *
 *  Author: Zverintsev Dima
 * 
 *  Creates and store any kind of camera in the array
 *  Currently supports android camera only
 *  Store information about currently selected camera
 */


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

    // Used to direct camera access
    CustomCamera* operator->() {return selected;};

    inline const char* get_dcim() const {return dcim;};
    /*
        Add new kind of camera
        for android camera use kind = "android"
    */
    void add_camera_kindof(jstring kind);
    /*
      Select a camera from array
      'id' is an array index
    */
    void select_camera(uint32_t id);
    
private:
    JNIEnv* env;
    jobject instance;
    char* dcim = nullptr;
    CustomCamera* selected = nullptr;
};

#endif