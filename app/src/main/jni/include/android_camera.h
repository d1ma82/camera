#ifndef _CAMERA_MANAGER_H
#define _CAMERA_MANAGER_H

#include <camera/NdkCameraManager.h>
#include <android/native_window_jni.h>
#include <functional>

using native_window_ptr = std::unique_ptr<ANativeWindow, std::function<void(ANativeWindow*)>>;

class NDKCamera {
private:
    camera_status_t status = ACAMERA_OK;

    ACameraManager* manager = nullptr;
    ACameraIdList* id_list = nullptr;
    ACameraDevice* device = nullptr;
    ACameraMetadata* metadata = nullptr;
    ACameraCaptureSession* session = nullptr;

    native_window_ptr camera_window = nullptr;  // android side preview 

    JNIEnv* env = nullptr;
    const  char* cam_id = nullptr;
    // sequence number from capture session
    int sequence = 0;

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

    static void instance(JNIEnv* env);
    static void select_camera(const char* id);
    static void best_resolution(int32_t in_out_resolution[2]);
    static void repeat(jobject surface);
};

#endif