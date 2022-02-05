#ifndef _CAMERA_MANAGER_H
#define _CAMERA_MANAGER_H

#include <camera/NdkCameraManager.h>
#include <android/native_window_jni.h>

class NDKCamera {
private:
    camera_status_t status = ACAMERA_OK;

    ACameraManager* manager = nullptr;
    ACameraIdList* id_list = nullptr;
    ACameraMetadata* metadata = nullptr;

    JNIEnv* env = nullptr;

    NDKCamera(){};
    NDKCamera(JNIEnv* env);
    static const char* error_message(camera_status_t status);
public:

   // copy-move disabled
    NDKCamera(const NDKCamera&) = delete;
    NDKCamera(NDKCamera&&) = delete;
    NDKCamera& operator=(const NDKCamera&) = delete;
    NDKCamera& operator=(NDKCamera&&) = delete;
    ~NDKCamera();  

    static void new_instance(JNIEnv* env);
    static void select_camera(const char* id);
};

#endif