#include "log.h"
#include "camera_engine.h"
#include <android/native_window_jni.h>

CameraEngine::CameraEngine(JNIEnv* env, jobject instance, jstring facing, jint w, jint h):
    env(env), 
    instance(instance), 
    requested_facing(facing),
    requested_width(w), 
    requested_height(h), 
    surface(nullptr), 
    camera(nullptr) {

    jboolean is_copy;
    const char* camera_facing_ = env->GetStringUTFChars(facing, &is_copy);
    camera = new NDKCamera(camera_facing_, requested_width, requested_height);
    env->ReleaseStringUTFChars(facing, camera_facing_);
}

CameraEngine::~CameraEngine() {

    if (camera) {
        delete camera;
        camera = nullptr;
    }

    if (surface) {
        env->DeleteGlobalRef(surface);
        surface = nullptr;
    }
}

void CameraEngine::create_session(jobject surface){

    this->surface = env->NewGlobalRef(surface);
    camera->create_session(ANativeWindow_fromSurface(env, surface));
}

void CameraEngine::start_preview(bool start){
    camera->start_preview(start);
}

const jintArray CameraEngine::get_compatible_res() const{

    int32_t arr[2];
    jintArray ret = env->NewIntArray(2);
    camera->compatible_preview_size(arr);
    env->SetIntArrayRegion(ret, 0, 2, arr);
    return ret;
}

int32_t CameraEngine::get_sensor_orientation(){
    return 0;
}

jobject CameraEngine::get_surface(){
    return nullptr;}