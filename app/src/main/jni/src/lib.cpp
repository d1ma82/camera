#include "log.h"
#include "camera_engine.h"
#include <jni.h>
#include <string>

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
 *  Create a new instatnse of camera
 *  @param camera_id define camera id ie. for android("back", "front")
 *  @param width requested width
 *  @param height requested height
 *  @return handle to camera engine
*/
extern "C" JNIEXPORT jlong JNICALL Java_com_home_camera_CameraWrapper_create(
    JNIEnv* env, 
    jobject instance,
    jstring camera_facing
) {
    engine = new CameraEngine(env, instance, camera_facing);
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

extern "C" JNIEXPORT void JNICALL Java_com_home_camera_CameraWrapper_onPreviewSurfaceCreated(
    JNIEnv* env, 
    jobject instance,
    jlong cam_obj,
    jobject surface
) {
    ASSERT(cam_obj && (jlong)engine == cam_obj, "NativeObject should not be null Pointer")
    CameraEngine* app = reinterpret_cast<CameraEngine*>(cam_obj);
    app->create_session(surface);
    app->start_preview(true);
}

extern "C" JNIEXPORT void JNICALL Java_com_home_camera_CameraWrapper_onPreviewSurfaceDestroyed(
    JNIEnv* env, 
    jobject instance,
    jlong cam_obj,
    jobject surface   
) {
    ASSERT(cam_obj && (jlong)engine == cam_obj, "NativeObject should not be null Pointer")
    CameraEngine* app = reinterpret_cast<CameraEngine*>(cam_obj);
    app->start_preview(false);
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
    ASSERT(cam_obj && (jlong)engine == cam_obj, "NativeObject should not be null Pointer")
    CameraEngine* app = reinterpret_cast<CameraEngine*>(cam_obj);
    return app->get_compatible_res(width, height);
}

        // With Open GL surface

extern "C" JNIEXPORT void Java_com_home_camera_CameraWrapper_onSurfaceCreated(
    JNIEnv* env, 
    jobject instance,
    jlong cam_obj,
    jint texture_id
) {
    ASSERT(cam_obj && (jlong)engine == cam_obj, "NativeObject should not be null Pointer")
    CameraEngine* app = reinterpret_cast<CameraEngine*>(cam_obj);
    app->init_surface(texture_id);
}

extern "C" JNIEXPORT void Java_com_home_camera_CameraWrapper_onSurfaceChanged(
    JNIEnv* env, 
    jobject instance,
    jlong cam_obj,
    jobject surface
) {
    ASSERT(cam_obj && (jlong)engine == cam_obj, "NativeObject should not be null Pointer")
    CameraEngine* app = reinterpret_cast<CameraEngine*>(cam_obj);

    app->create_session(surface);
    app->start_preview(true);
}

extern "C" JNIEXPORT void Java_com_home_camera_CameraWrapper_onDrawFrame(
    JNIEnv* env, 
    jobject instance,
    jlong cam_obj,
    jint width,
    jint height,
    jfloatArray texMatArray
) {
    ASSERT(cam_obj && (jlong)engine == cam_obj, "NativeObject should not be null Pointer")
    CameraEngine* app = reinterpret_cast<CameraEngine*>(cam_obj);
    app->draw_frame(width, height, texMatArray);
}

