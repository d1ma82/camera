#include "android_camera.h"
#include "log.h"
#include <memory>
#include <vector>
#include <media/NdkImage.h>

using capture_request_ptr = std::unique_ptr<ACaptureRequest, std::function<void(ACaptureRequest*)>>;
using camera_output_target_ptr = std::unique_ptr<ACameraOutputTarget, std::function<void(ACameraOutputTarget*)>>;
using capture_session_output_container_ptr = std::unique_ptr<ACaptureSessionOutputContainer, std::function<void(ACaptureSessionOutputContainer*)>>;
using capture_session_output_ptr = std::unique_ptr<ACaptureSessionOutput, std::function<void(ACaptureSessionOutput*)>>;

std::unique_ptr<NDKCamera> camera = nullptr;

void onDisconnected(void* context, ACameraDevice* device){
    LOGI("Device disconnected");
}
void onError(void* context, ACameraDevice* device, int error){
    LOGI("Device error, %d", error);
}

static ACameraDevice_stateCallbacks cameraDeviceCallbacks = {
        cameraDeviceCallbacks.context = nullptr,
        cameraDeviceCallbacks.onDisconnected = onDisconnected,
        cameraDeviceCallbacks.onError = onError,
};

void onSessionActive(void* context, ACameraCaptureSession *session) {}
void onSessionReady(void* context, ACameraCaptureSession *session) {}
void onSessionClosed(void* context, ACameraCaptureSession *session) {}

static ACameraCaptureSession_stateCallbacks sessionStateCallbacks {
        sessionStateCallbacks.context = nullptr,
        sessionStateCallbacks.onActive = onSessionActive,
        sessionStateCallbacks.onReady = onSessionReady,
        sessionStateCallbacks.onClosed = onSessionClosed
};

void onCaptureFailed(void* context, ACameraCaptureSession* session, ACaptureRequest* request, ACameraCaptureFailure* failure) {
    LOGI("onCaptureFailed %d", failure->reason);
}
void onCaptureSequenceCompleted(void* context, ACameraCaptureSession* session, int sequenceId, int64_t frameNumber){}
void onCaptureSequenceAborted(void* context, ACameraCaptureSession* session, int sequenceId){}
void onCaptureCompleted (
        void* context, ACameraCaptureSession* session,
        ACaptureRequest* request, const ACameraMetadata* result){

    // TODO: add image output to android
}

static ACameraCaptureSession_captureCallbacks captureCallbacks {
        captureCallbacks.context = nullptr,
        captureCallbacks.onCaptureStarted = nullptr,
        captureCallbacks.onCaptureProgressed = nullptr,
        captureCallbacks.onCaptureCompleted = onCaptureCompleted,
        captureCallbacks.onCaptureFailed = onCaptureFailed,
        captureCallbacks.onCaptureSequenceCompleted = onCaptureSequenceCompleted,
        captureCallbacks.onCaptureSequenceAborted = onCaptureSequenceAborted,
        captureCallbacks.onCaptureBufferLost = nullptr,
};



NDKCamera::NDKCamera(JNIEnv* env) {

    this->env = env;
    manager = ACameraManager_create();
    status = ACameraManager_getCameraIdList(manager, &id_list);
    LOGI("Camera count %d, %s", id_list->numCameras, error_message(status));
}

NDKCamera::~NDKCamera() {

    if (session) {

        status = ACameraCaptureSession_abortCaptures(session);
        status = ACameraCaptureSession_stopRepeating(session);
        ACameraCaptureSession_close(session);
        session = nullptr;
        LOGI("Close session, %s", error_message(status));
    } 

    if (metadata) ACameraMetadata_free(metadata);
    metadata = nullptr;

    if (device) ACameraDevice_close(device);
    device = nullptr;

    if (id_list) ACameraManager_deleteCameraIdList(id_list);
    id_list = nullptr;

    if (manager) ACameraManager_delete(manager);
    manager = nullptr;
}

void NDKCamera::instance(JNIEnv* env) {

    if (camera == nullptr) camera.reset(new NDKCamera(env));
}

void NDKCamera::select_camera(const char* id) {

    acamera_metadata_enum_acamera_lens_facing select = ACAMERA_LENS_FACING_BACK;
    if (strncmp(id, "back", 1) == 0){
            select = ACAMERA_LENS_FACING_BACK;
    }
    else if (strncmp(id, "front", 1) == 0) {
            select = ACAMERA_LENS_FACING_FRONT;
    }

    for (int i=0; i<camera->id_list->numCameras; i++) {
        
        ACameraManager_getCameraCharacteristics(
            camera->manager, camera->id_list->cameraIds[i], &camera->metadata);

        ACameraMetadata_const_entry lens = {0};
        ACameraMetadata_getConstEntry(camera->metadata, ACAMERA_LENS_FACING, &lens);
        auto facing = (acamera_metadata_enum_acamera_lens_facing) lens.data.u8[0];

        if ((facing == select) && (camera->status == ACAMERA_OK)) {

            camera->cam_id = camera->id_list->cameraIds[i];
            camera->status = ACameraManager_openCamera(
                camera->manager, 
                camera->cam_id, 
                &cameraDeviceCallbacks, 
                &camera->device);
            LOGI("Camera facing %s, %d", id, camera->status);
            break;
        }
        ACameraMetadata_free(camera->metadata);
    }
}

void NDKCamera::best_resolution(int32_t in_out_resolution[2]) {

    LOGI("Input %d x %d", in_out_resolution[0], in_out_resolution[1]);
    ACameraMetadata_const_entry entry = {0};
    // format, width, height, input?
    ACameraMetadata_getConstEntry(camera->metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry);
    int32_t window_square = in_out_resolution[0] * in_out_resolution[1];
    int32_t min = 0;
    int32_t j = 0;

    for (int i=0; i<entry.count; i+=4) {

        int32_t input = entry.data.i32[i+3];
        if (input) continue;  // intresting output
        
        int32_t format = entry.data.i32[i+0];
        if (format == AIMAGE_FORMAT_RGBA_8888) {    

            int32_t diff = entry.data.i32[i+1] * entry.data.i32[i+2] - window_square;
            if (min == 0) {min = diff; j = i;}
            else if ((diff > 0) && (diff < min)) {

                  min = diff;
                  j = i;
            }
            LOGI("AVAILABLE_STREAM_CONFIGURATIONS, %d x %d", entry.data.i32[i+1], entry.data.i32[i+2]);
        }
    }
    in_out_resolution[0] = entry.data.i32[j+1];
    in_out_resolution[1] = entry.data.i32[j+2];
    LOGI("Best preview %d x %d", in_out_resolution[0], in_out_resolution[1]);
}

void NDKCamera::repeat(jobject surface) {

    camera->camera_window = native_window_ptr {
        ANativeWindow_fromSurface(camera->env, surface), ANativeWindow_release};
    
    int32_t width = ANativeWindow_getWidth(camera->camera_window.get());
    int32_t height = ANativeWindow_getHeight(camera->camera_window.get());
    LOGI("Repeat resolution %d x %d", width, height);

    auto image_target = camera_output_target_ptr {
        [] () {
                ACameraOutputTarget* target {};
                camera->status = ACameraOutputTarget_create(camera->camera_window.get(), &target);
                return target;
        }(), ACameraOutputTarget_free
    };

    auto request = capture_request_ptr { 
        [] () {
                ACaptureRequest* request {};
                camera->status = ACameraDevice_createCaptureRequest(camera->device, TEMPLATE_PREVIEW, &request);
                return request;
        }(), ACaptureRequest_free
    };
    ACaptureRequest_addTarget(request.get(), image_target.get());

    auto container = capture_session_output_container_ptr { 
        [] () {
                ACaptureSessionOutputContainer* container {};
                camera->status = ACaptureSessionOutputContainer_create(&container);
                return container;
        } (), ACaptureSessionOutputContainer_free
    };

    auto image_output = capture_session_output_ptr {
        [] () {
                ACaptureSessionOutput* output {};
                camera->status = ACaptureSessionOutput_create(camera->camera_window.get(), &output);
                return output;
        }(), ACaptureSessionOutput_free
    };
    ACaptureSessionOutputContainer_add(container.get(), image_output.get());

    if (camera->status == ACAMERA_OK) {
        ACameraDevice_createCaptureSession(
            camera->device, container.get(), &sessionStateCallbacks, &camera->session);

        ACaptureRequest* _request = request.get();
        captureCallbacks.context = camera.get();
        camera->status = ACameraCaptureSession_setRepeatingRequest(
            camera->session, &captureCallbacks, 1, &_request, &camera->sequence);
    }
    ACaptureSessionOutputContainer_remove(container.get(), image_output.get());
    ACaptureRequest_removeTarget(request.get(), image_target.get());
    LOGI("Repeating request status %d", camera->status);
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