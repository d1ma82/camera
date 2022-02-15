#include "log.h"
#include "camera_manager.h"
#include "opengl.h"
#include <memory>
#include <media/NdkImage.h>

static int32_t screen_width = 0, screen_height = 0;
static bool session_created = false, preview_started = false;

void OnCameraAvailable(void* ctx, const char* id) {}
void OnCameraUnavailable(void* ctx, const char* id) {}

static ACameraManager_AvailabilityCallbacks cameraMgrListener = {
      cameraMgrListener.context = nullptr,
      cameraMgrListener.onCameraAvailable = OnCameraAvailable,
      cameraMgrListener.onCameraUnavailable = OnCameraUnavailable,
  };

void onDisconnected(void* context, ACameraDevice* device){
    LOGI("Device disconnected")
}
void onError(void* context, ACameraDevice* device, int error){
    LOGI("Device error, %d", error)
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
    LOGI("onCaptureFailed %d", failure->reason)
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

NDKCamera::NDKCamera(const char* facing) {

    manager = ACameraManager_create();
    CALL(ACameraManager_registerAvailabilityCallback(manager, &cameraMgrListener));
    CALL(ACameraManager_getCameraIdList(manager, &id_list));
    CALL(ACaptureSessionOutputContainer_create(&container));
    requests.resize(REQUESTS_COUNT);
    memset(requests.data(), 0, requests.size() * sizeof(RequestInfo));
    LOGI("Camera count: %d", id_list->numCameras)
    select_camera(facing);
}

NDKCamera::~NDKCamera() {

    LOGI("NDKCamera destroy")
    
    CALL(ACameraCaptureSession_stopRepeating(session))
    ACameraCaptureSession_close(session);

    requests.resize(0);

    ogl::destroy();

    if (container) ACaptureSessionOutputContainer_free(container);
    container = nullptr;

    if (metadata) ACameraMetadata_free(metadata);
    metadata = nullptr;

    if (device) CALL(ACameraDevice_close(device));
    device = nullptr;

    if (id_list) ACameraManager_deleteCameraIdList(id_list);
    id_list = nullptr;

    if (manager) ACameraManager_delete(manager);
    manager = nullptr;
    LOGI("NDKCamera destroyed")
}

void NDKCamera::select_camera(const char* facing) noexcept {

    if (strncmp(facing, "back", 1) == 0){
            this->facing = ACAMERA_LENS_FACING_BACK;
    }
    else if (strncmp(facing, "front", 1) == 0) {
            this->facing = ACAMERA_LENS_FACING_FRONT;
    }

    for (int i=0; i<id_list->numCameras; i++) {
        
        CALL(ACameraManager_getCameraCharacteristics(
            manager, id_list->cameraIds[i], &metadata));

        ACameraMetadata_const_entry lens = {0};
        CALL(ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_FACING, &lens));
        auto cam_facing = (acamera_metadata_enum_acamera_lens_facing) lens.data.u8[0];

        if ((cam_facing == this->facing) && (status == ACAMERA_OK)) {

            cam_id = id_list->cameraIds[i];
            CALL(ACameraManager_openCamera(manager, cam_id, &cameraDeviceCallbacks, &device))
            LOGI("Camera facing %s, %d", facing, status)
            break;
        }
        ACameraMetadata_free(metadata); // do not remove!
    }
}

void NDKCamera::create_session(ANativeWindow* window) noexcept {

    if (session_created) return;
    ANativeWindow_acquire(window);
   // RequestInfo* info = new RequestInfo;
    requests[PREVIEW_IDX].req_template = TEMPLATE_PREVIEW;
    requests[PREVIEW_IDX].window = native_window{window, ANativeWindow_release};

    requests[PREVIEW_IDX].output = session_output {
        [window, this] () {
                ACaptureSessionOutput* output {};
                CALL(ACaptureSessionOutput_create(window, &output))
                return output;
        }(), ACaptureSessionOutput_free
    };
    ACaptureSessionOutputContainer_add(container, requests[PREVIEW_IDX].output.get());

    requests[PREVIEW_IDX].target = output_target {
        [window, this] () {
                ACameraOutputTarget* target {};
                CALL(ACameraOutputTarget_create(window, &target))
                return target;
        }(), ACameraOutputTarget_free
    };

    requests[PREVIEW_IDX].request = capture_request {
        [this] (ACameraDevice_request_template request_template) {
            ACaptureRequest* request {};
            CALL(ACameraDevice_createCaptureRequest(device, request_template, &request))
            return request;
        } (requests[PREVIEW_IDX].req_template), ACaptureRequest_free
    };
    CALL(ACaptureRequest_addTarget(requests[PREVIEW_IDX].request.get(), requests[PREVIEW_IDX].target.get()))
    CALL(ACameraDevice_createCaptureSession(device, container, &sessionStateCallbacks, &session))
    session_created = true; 
    LOGI("Preview session created, %d", status)
}

void NDKCamera::start_preview(bool start) noexcept {

    if (start) {

        if (preview_started) return;
        ACaptureRequest* request = requests[PREVIEW_IDX].request.get();
        CALL(ACameraCaptureSession_setRepeatingRequest(
            session, &captureCallbacks, 1, &request, &requests[PREVIEW_IDX].session_sequence))  
        preview_started = true; 
    } else {
        
        CALL(ACameraCaptureSession_stopRepeating(session))
        preview_started = false;
    }
}

void NDKCamera::calc_compatible_preview_size(int32_t width, int32_t height, int32_t out_compatible_res[2]) noexcept {

    screen_width = width; screen_height = height;
    ACameraMetadata_const_entry entry = {0};
    // format, width, height, input?
    CALL(ACameraMetadata_getConstEntry(metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry));
 
    int32_t window_square = width * height;
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
            LOGI("AVAILABLE_STREAM_CONFIGURATIONS, %d x %d", entry.data.i32[i+1], entry.data.i32[i+2])
        }
    }    
    out_compatible_res[0] = entry.data.i32[j+1];
    out_compatible_res[1] = entry.data.i32[j+2];
    LOGI("Camera compatible resolution, %d x %d", out_compatible_res[0], out_compatible_res[1])
}

void NDKCamera::init_surface(int32_t texture_id) noexcept {

    ogl::init_surface(screen_width, screen_height, texture_id);
}

void NDKCamera::draw_frame(const float texture_mat[]) {

    ogl::draw_frame(texture_mat);
}

void NDKCamera::next_shader() noexcept {
    ogl::next_shader();
}
