#include "log.h"
#include "camera_engine.h"
#include <android/native_window_jni.h>

CameraEngine::CameraEngine(JNIEnv* env, jobject instance, jstring facing, jstring in_dcim):
    env(env), 
    instance(instance), 
    requested_facing(facing),
    camera(nullptr) {

    jboolean is_copy;
    const char* dcim_ = env->GetStringUTFChars(in_dcim, &is_copy);
    dcim = std::string(dcim_);
    env->ReleaseStringUTFChars(in_dcim, dcim_);

    const char* camera_facing_ = env->GetStringUTFChars(facing, &is_copy);
    camera = new NDKCamera(camera_facing_);
    env->ReleaseStringUTFChars(facing, camera_facing_);
}

CameraEngine::~CameraEngine() {

    if (camera) {
        delete camera;
        camera = nullptr;
    }
}