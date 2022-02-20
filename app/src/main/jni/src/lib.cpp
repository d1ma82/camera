#include "log.h"
#include <jni.h>
#include "camera_engine.h"
#include <string>
#include <android/native_window_jni.h>

CameraEngine* engine = nullptr;

/** 
 * Test function to check lib load
*/
extern "C" JNIEXPORT jstring JNICALL Java_com_home_camera_CameraWrapper_greeting(
    JNIEnv* env, 
    jobject instance,
    jstring name
) {
    jboolean is_copy;
    const char* name_ = env->GetStringUTFChars(name, &is_copy);
    std::string complete = "Hello " + (std::string)name_ + '!';
    env->ReleaseStringUTFChars(name, name_);

    return  env->NewStringUTF(complete.c_str());
}

/** 
 *  Create a new instatnse of camera and select default camera with index 0
 *  @param kind define what camera kind of ie. "android"
 *  @param dcim photo storage directory
 *  @return handle to camera engine
*/
extern "C" JNIEXPORT jlong JNICALL Java_com_home_camera_CameraWrapper_create(
    JNIEnv* env, 
    jobject instance,
    jstring kind,
    jstring dcim
) {
    engine = new CameraEngine(env, instance, dcim);
    engine->add_camera_kindof(kind);
    return reinterpret_cast<jlong>(engine);
}

/**
 *   releases native application object, which
 *   triggers native camera object be released
 */
extern "C" JNIEXPORT void JNICALL Java_com_home_camera_CameraWrapper_delete(
    JNIEnv* env, 
    jobject instance,
    jlong cam_obj
) {
    CameraEngine* app = reinterpret_cast<CameraEngine*>(cam_obj);
    if ((app != nullptr) && (engine == app)) {

        delete engine;
        engine = nullptr;
    }
}

/**
 *      select camera with index
 *      
 */
extern "C" JNIEXPORT void JNICALL Java_com_home_camera_CameraWrapper_selectCamera(
    JNIEnv* env, 
    jobject instance,
    jlong cam_obj,
    jint index
) {
    ASSERT(cam_obj && (jlong)engine == cam_obj, "selectCamera: NativeObject should not be null Pointer")
    CameraEngine* app = reinterpret_cast<CameraEngine*>(cam_obj);

    (*app)->start_preview(false);
    (*app)->close_session();
    app->select_camera(index);
}

/**
 *      Launch new session
 *      When  surface created in android side
 *      
 */
extern "C" JNIEXPORT void JNICALL Java_com_home_camera_CameraWrapper_onPreviewSurfaceCreated(
    JNIEnv* env, 
    jobject instance,
    jlong cam_obj,
    jobject surface,
    jint texture_id
) {
    ASSERT(cam_obj && (jlong)engine == cam_obj, "onPreviewSurfaceCreated: NativeObject should not be null Pointer")
    CameraEngine* app = reinterpret_cast<CameraEngine*>(cam_obj);
    if (texture_id > 0) (*app)->init_surface(texture_id);
    (*app)->create_session(ANativeWindow_fromSurface(env, surface));
    (*app)->start_preview(true);
}

/** 
 * Find best preview resolution by the given width and height
*/
extern "C" JNIEXPORT jintArray JNICALL Java_com_home_camera_CameraWrapper_compatibleResolution(
    JNIEnv* env, 
    jobject instance,
    jlong cam_obj,
    jint width,
    jint height
) {
    ASSERT(cam_obj && (jlong)engine == cam_obj, "compatibleResolution: NativeObject should not be null Pointer")
    CameraEngine* app = reinterpret_cast<CameraEngine*>(cam_obj);

    int32_t arr[2];
    jintArray ret = env->NewIntArray(2);
    (*app)->calc_compatible_preview_size(width, height, arr);
    env->SetIntArrayRegion(ret, 0, 2, arr);
    return ret;
}

        // With Open GL surface

extern "C" JNIEXPORT void JNICALL Java_com_home_camera_CameraWrapper_onDrawFrame(
    JNIEnv* env, 
    jobject instance,
    jlong cam_obj
) {
    ASSERT(cam_obj && (jlong)engine == cam_obj, "onDrawFrame: NativeObject should not be null Pointer")
    CameraEngine* app = reinterpret_cast<CameraEngine*>(cam_obj);
    (*app)->draw_frame();
}

extern "C" JNIEXPORT void JNICALL Java_com_home_camera_CameraWrapper_nextShader(
    JNIEnv* env, 
    jobject instance,
    jlong cam_obj
) {
    ASSERT(cam_obj && (jlong)engine == cam_obj, "nextShader: NativeObject should not be null Pointer")
    CameraEngine* app = reinterpret_cast<CameraEngine*>(cam_obj);
    (*app)->next_shader();
}

extern "C" JNIEXPORT void JNICALL Java_com_home_camera_CameraWrapper_takePhoto(
    JNIEnv* env, 
    jobject instance,
    jlong cam_obj
) {
    ASSERT(cam_obj && (jlong)engine == cam_obj, "takePhoto: NativeObject should not be null Pointer")
    CameraEngine* app = reinterpret_cast<CameraEngine*>(cam_obj);
    
    (*app)->take_photo(app->get_dcim());
}
