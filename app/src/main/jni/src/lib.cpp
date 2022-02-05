#include "android_camera.h"
#include <jni.h>
#include <string>

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
    std::string complete = "Hello " + (std::string) name_ + '!';
    env->ReleaseStringUTFChars(name, name_);

    return  env->NewStringUTF(complete.c_str());
}

/** 
 *  Create a new instatnse of camera
 *  @param camera_type define witch camera to use (android ,ip, other...)
 *  @param camera_id define camera id ie. (android "back", "front")
 *  @return if android camera count
*/
extern "C" JNIEXPORT jint JNICALL Java_com_home_camera_CameraWrapper_select(
    JNIEnv* env, 
    jobject instance,
    jstring camera_type,
    jstring camera_id
) {
    jboolean is_copy;
    const char* camera_type_ = env->GetStringUTFChars(camera_type, &is_copy);
    if (strncmp(camera_type_, "android", 1) == 0) {

        NDKCamera::new_instance(env);
        const char* camera_id_ = env->GetStringUTFChars(camera_id, &is_copy);
        NDKCamera::select_camera(camera_id_);
        env->ReleaseStringUTFChars(camera_id, camera_id_);
    }
    env->ReleaseStringUTFChars(camera_type, camera_type_);
    return 0;
}