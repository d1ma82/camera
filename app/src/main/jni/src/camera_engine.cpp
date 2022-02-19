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

void CameraEngine::create_session(jobject surface){

   // this->surface = env->NewGlobalRef(surface);
    camera->create_session(ANativeWindow_fromSurface(env, surface));
}

void CameraEngine::init_surface(int32_t texture_id) {

    camera->init_surface(texture_id);
}

void CameraEngine::next_shader() {
    camera->next_shader();
}

void CameraEngine::take_photo() {

    camera->take_photo(dcim.c_str());
}

void CameraEngine::start_preview(bool start){

    camera->start_preview(start);
}

void CameraEngine::draw_frame() {

   // float* arr = env->GetFloatArrayElements(transform_mat, 0);
    camera->draw_frame();
   // env->ReleaseFloatArrayElements(transform_mat, arr, 0);
}

const jintArray CameraEngine::get_compatible_res(jint width, jint height) const{

    int32_t arr[2];
    jintArray ret = env->NewIntArray(2);
    camera->calc_compatible_preview_size(width, height, arr);
    env->SetIntArrayRegion(ret, 0, 2, arr);
    return ret;
}

int32_t CameraEngine::get_sensor_orientation(){
    return 0;
}