#include <jni.h>
#include <string>

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