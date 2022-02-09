#include "camera_manager.h"
#include "log.h"
#include <memory>
#include <media/NdkImage.h>

int32_t compatible_res[2] {0,0};  // best preview size

void OnCameraAvailable(void* ctx, const char* id) {}
void OnCameraUnavailable(void* ctx, const char* id) {}

static ACameraManager_AvailabilityCallbacks cameraMgrListener = {
      cameraMgrListener.context = nullptr,
      cameraMgrListener.onCameraAvailable = OnCameraAvailable,
      cameraMgrListener.onCameraUnavailable = OnCameraUnavailable,
  };

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
        ACaptureRequest* request, const ACameraMetadata* result){}

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

NDKCamera::NDKCamera(const char* facing, int32_t requested_width, int32_t requested_height) {

    manager = ACameraManager_create();
    ACameraManager_registerAvailabilityCallback(manager, &cameraMgrListener);
    status = ACameraManager_getCameraIdList(manager, &id_list);
    status = ACaptureSessionOutputContainer_create(&container);
    select_camera(facing);
    calc_compatible_preview_size(requested_width, requested_height);
}

NDKCamera::~NDKCamera() {

    LOGI("NDKCamera destroy");
    
    ACameraCaptureSession_stopRepeating(session);
    ACameraCaptureSession_close(session);

    for (auto req: requests) {
        ACaptureRequest_removeTarget(req->request.get(), req->target.get());
        ACaptureSessionOutputContainer_remove(container, req->output.get());
        delete req;
    }

    if (container) ACaptureSessionOutputContainer_free(container);
    container = nullptr;

    if (metadata) ACameraMetadata_free(metadata);
    metadata = nullptr;

    if (device) ACameraDevice_close(device);
    device = nullptr;

    if (id_list) ACameraManager_deleteCameraIdList(id_list);
    id_list = nullptr;

    if (manager) ACameraManager_delete(manager);
    manager = nullptr;
    LOGI("NDKCamera destroyed");
}

void NDKCamera::select_camera(const char* facing) noexcept {

    if (strncmp(facing, "back", 1) == 0){
            this->facing = ACAMERA_LENS_FACING_BACK;
    }
    else if (strncmp(facing, "front", 1) == 0) {
            this->facing = ACAMERA_LENS_FACING_FRONT;
    }

    for (int i=0; i<id_list->numCameras; i++) {
        
        ACameraManager_getCameraCharacteristics(
            manager, id_list->cameraIds[i], &metadata);

        ACameraMetadata_const_entry lens = {0};
        ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_FACING, &lens);
        auto cam_facing = (acamera_metadata_enum_acamera_lens_facing) lens.data.u8[0];

        if ((cam_facing == this->facing) && (status == ACAMERA_OK)) {

            cam_id = id_list->cameraIds[i];
            status = ACameraManager_openCamera(manager, cam_id, &cameraDeviceCallbacks, &device);
            LOGI("Camera facing %s, %d", facing, status);
            break;
        }
        ACameraMetadata_free(metadata); // do not remove!
    }
}

void NDKCamera::create_session(ANativeWindow* window) noexcept {

    LOGI("Session preview window size %d x %d", compatible_res[0], compatible_res[1]);

    ANativeWindow_acquire(window);
    RequestInfo* info = new RequestInfo;
    info->req_template = TEMPLATE_PREVIEW;
    info->window = native_window{window, ANativeWindow_release};

    info->output = session_output {
        [window, this] () {
                ACaptureSessionOutput* output {};
                this->status = ACaptureSessionOutput_create(window, &output);
                return output;
        }(), ACaptureSessionOutput_free
    };
    ACaptureSessionOutputContainer_add(container, info->output.get());

    info->target = output_target {
        [window, this] () {
                ACameraOutputTarget* target {};
                this->status = ACameraOutputTarget_create(window, &target);
                return target;
        }(), ACameraOutputTarget_free
    };

    info->request = capture_request {
        [this, info]() {
            ACaptureRequest* request {};
            this->status = ACameraDevice_createCaptureRequest(device, info->req_template, &request);
            return request;
        }(), ACaptureRequest_free
    };
    ACaptureRequest_addTarget(info->request.get(), info->target.get());
    requests.push_back(info);
    status = ACameraDevice_createCaptureSession(device, container, &sessionStateCallbacks, &session);
}

void NDKCamera::start_preview(bool start) noexcept {

    if (start) {
        ACaptureRequest* request = requests[0]->request.get();
        status = ACameraCaptureSession_setRepeatingRequest(
            session, &captureCallbacks, 1, &request, &requests[0]->session_sequence);
    } else {
        ACameraCaptureSession_stopRepeating(session);
    }
}

void NDKCamera::calc_compatible_preview_size(int32_t requested_width, int32_t requested_height) noexcept {

    ACameraMetadata_const_entry entry = {0};
    // format, width, height, input?
    ACameraMetadata_getConstEntry(metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry);
    int32_t window_square = requested_width * requested_height;
    int32_t min = 0;
    int32_t j = 0;

    for (int i=0; i<entry.count; i+=4) {

        int32_t input = entry.data.i32[i+3];
        if (input) continue;  // intresting output
        
        int32_t format = entry.data.i32[i+0];
        if (format == AIMAGE_FORMAT_YUV_420_888) {

            int32_t diff = entry.data.i32[i+1] * entry.data.i32[i+2] - window_square;
            if (min == 0) {min = diff; j = i;}
            else if ((diff > 0) && (diff < min)) {

                  min = diff;
                  j = i;
            }
            LOGI("AVAILABLE_STREAM_CONFIGURATIONS, %d x %d", entry.data.i32[i+1], entry.data.i32[i+2]);
        }
    }
    compatible_res[0] = entry.data.i32[j+1];
    compatible_res[1] = entry.data.i32[j+2];
    LOGI("Best preview %d x %d", compatible_res[0], compatible_res[1]);
}

void NDKCamera::compatible_preview_size(int32_t compatible_resolution[2]) noexcept {

    compatible_resolution[0] = compatible_res[0];
    compatible_resolution[1] = compatible_res[1];
}