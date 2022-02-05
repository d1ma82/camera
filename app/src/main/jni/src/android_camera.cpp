#include "android_camera.h"
#include "log.h"
#include <memory>

std::unique_ptr<NDKCamera> camera = nullptr;


NDKCamera::NDKCamera(JNIEnv* env) {

    this->env = env;
    manager = ACameraManager_create();
    ACameraManager_getCameraIdList(manager, &id_list);
    LOGI("Camera count %d, %s", id_list->numCameras, error_message(status));
}

NDKCamera::~NDKCamera() {

    if (id_list) ACameraManager_deleteCameraIdList(id_list);
    id_list = nullptr;

    if (manager) ACameraManager_delete(manager);
    manager = nullptr;
}

void NDKCamera::new_instance(JNIEnv* env) {

    if (camera == nullptr) camera.reset(new NDKCamera(env));
}

void NDKCamera::select_camera(const char* id) {

    LOGI("Select not implemented yet");
}

const char* NDKCamera::error_message(camera_status_t status) {

 switch (status) {
        case ACAMERA_OK : return "OK";
        case ACAMERA_ERROR_UNKNOWN:
            return "Camera operation has failed due to an unspecified cause.";
        case ACAMERA_ERROR_INVALID_PARAMETER:
            return "Camera operation has failed due to an invalid parameter being "
                "passed to the method.";
        case ACAMERA_ERROR_CAMERA_DISCONNECTED:
            return "Camera operation has failed because the camera device has been "
                "closed, possibly because a higher-priority client has taken "
                "ownership of the camera device.";
        case ACAMERA_ERROR_NOT_ENOUGH_MEMORY:
            return "Camera operation has failed due to insufficient memory.";
        case ACAMERA_ERROR_METADATA_NOT_FOUND:
            return "Camera operation has failed due to the requested metadata tag "
                "cannot be found in input. ACameraMetadata or ACaptureRequest";
        case ACAMERA_ERROR_CAMERA_DEVICE:
            return "Camera operation has failed and the camera device has "
                "encountered a fatal error and needs to be re-opened before it "
                "can be used again.";
        case ACAMERA_ERROR_CAMERA_SERVICE:
            /**
             * Camera operation has failed and the camera service has encountered a
             * fatal error.
             *
             * <p>The Android device may need to be shut down and restarted to
             * restore camera function, or there may be a persistent hardware
             * problem.</p>
             *
             * <p>An attempt at recovery may be possible by closing the
             * ACameraDevice and the ACameraManager, and trying to acquire all
             * resources again from scratch.</p>
             */
            return "Camera operation has failed and the camera service has "
                "encountered a fatal error.";
        case ACAMERA_ERROR_SESSION_CLOSED:
            return "The ACameraCaptureSession has been closed and cannot perform "
                "any operation other than ACameraCaptureSession_close.";
        case ACAMERA_ERROR_INVALID_OPERATION:
            return "Camera operation has failed due to an invalid internal "
                "operation. Usually this is due to a low-level problem that may "
                "resolve itself on retry";
        case ACAMERA_ERROR_STREAM_CONFIGURE_FAIL:
            return "Camera device does not support the stream configuration "
                "provided by application in ACameraDevice_createCaptureSession.";
        case ACAMERA_ERROR_CAMERA_IN_USE:
            return "Camera device is being used by another higher priority camera "
                "API client.";
        case ACAMERA_ERROR_MAX_CAMERA_IN_USE:
            return "The system-wide limit for number of open cameras or camera "
                "resources has been reached, and more camera devices cannot be "
                "opened until previous instances are closed.";
        case ACAMERA_ERROR_CAMERA_DISABLED:
            return "The camera is disabled due to a device policy, and cannot be "
                "opened.";
        case ACAMERA_ERROR_PERMISSION_DENIED:
            return "The application does not have permission to open camera.";
        default:
            return "Unknown exception";
        }   
}