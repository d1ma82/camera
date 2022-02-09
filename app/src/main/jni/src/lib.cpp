#include "log.h"
#include <jni.h>
#include <string>

#include "camera_engine.h"

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
 *  @param camera_type define witch camera to use ("android" ,"ip", other...)
 *  @param camera_id define camera id ie. for android("back", "front")
*/
extern "C" JNIEXPORT jlong JNICALL Java_com_home_camera_CameraWrapper_create(
    JNIEnv* env, 
    jobject instance,
    jstring camera_facing,
    jint width,
    jint height
) {
    engine = new CameraEngine(env, instance, camera_facing, width, height);
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
extern "C" JNIEXPORT jintArray JNICALL Java_com_home_camera_CameraWrapper_compatiblePreviewResolution(
    JNIEnv* env, 
    jobject instance,
    jlong cam_obj
) {
    ASSERT(cam_obj && (jlong)engine == cam_obj, "NativeObject should not be null Pointer")
    CameraEngine* app = reinterpret_cast<CameraEngine*>(cam_obj);
    return app->get_compatible_res();
}

extern "C" JNIEXPORT void Java_com_home_camera_CameraWrapper_onDrawFrame(
    JNIEnv* env, 
    jobject instance,
    jfloatArray texMatArray
) {
  //  float* tm = env->GetFloatArrayElements(texMatArray, 0);
  //  NDKCamera::on_draw(tm);
 //   env->ReleaseFloatArrayElements(texMatArray, tm, 0);
}

extern "C" JNIEXPORT void Java_com_home_camera_CameraWrapper_onSurfaceChange(
    JNIEnv* env, 
    jobject instance,
    jint width,
    jint height
) {
  //  NDKCamera::surface_changes(width, height);
}