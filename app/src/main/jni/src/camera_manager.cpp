#include "log.h"
#include "camera_manager.h"
#include "opengl.h"
#include <vector>
#include <dirent.h>
#include <thread>
#include <memory>
#include <string>

namespace NDKManager{
    static manager camera_manager = nullptr;
    static id_list camera_list = nullptr;
}

 static const char* dcim = nullptr;
 static std::vector<RequestInfo> requests;

static void OnCameraAvailable(void* ctx, const char* id) {}
static void OnCameraUnavailable(void* ctx, const char* id) {}

static ACameraManager_AvailabilityCallbacks cameraMgrListener = {
      cameraMgrListener.context = nullptr,
      cameraMgrListener.onCameraAvailable = OnCameraAvailable,
      cameraMgrListener.onCameraUnavailable = OnCameraUnavailable,
};

void NDKManager::manager_new_instance(){
    
    if (camera_manager == nullptr) {

        ACameraIdList* list = nullptr;
        camera_manager = {ACameraManager_create(), ACameraManager_delete};
        ACameraManager_registerAvailabilityCallback(camera_manager.get(), &cameraMgrListener);
        ACameraManager_getCameraIdList(camera_manager.get(), &list);
        camera_list = {list, ACameraManager_deleteCameraIdList};
        LOGI("Camera count: %d", camera_list -> numCameras)
    }
}

uint32_t NDKManager::count_camera() {

    return camera_list -> numCameras;
}

static void onDisconnected(void* context, ACameraDevice* device){
    LOGI("Device disconnected")
}
static void onError(void* context, ACameraDevice* device, int error){
    LOGI("Device error, %d", error)
}

static ACameraDevice_stateCallbacks cameraDeviceCallbacks = {
        cameraDeviceCallbacks.context = nullptr,
        cameraDeviceCallbacks.onDisconnected = onDisconnected,
        cameraDeviceCallbacks.onError = onError,
};

static void onSessionActive(void* context, ACameraCaptureSession *session) {}
static void onSessionReady(void* context, ACameraCaptureSession *session) {}
static void onSessionClosed(void* context, ACameraCaptureSession *session) {}

static ACameraCaptureSession_stateCallbacks sessionStateCallbacks {
        sessionStateCallbacks.context = nullptr,
        sessionStateCallbacks.onActive = onSessionActive,
        sessionStateCallbacks.onReady = onSessionReady,
        sessionStateCallbacks.onClosed = onSessionClosed
};

static void onCaptureFailed(void* context, ACameraCaptureSession* session, ACaptureRequest* request, ACameraCaptureFailure* failure) {
    LOGI("onCaptureFailed %d", failure->reason)
}
static void onCaptureSequenceCompleted(void* context, ACameraCaptureSession* session, int sequenceId, int64_t frameNumber){
    //resume preview
}
static void onCaptureSequenceAborted(void* context, ACameraCaptureSession* session, int sequenceId){}
static void onCaptureCompleted (
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

static void write_file(AImage* image) {

    int32_t w, h;
    AImage_getWidth(image, &w);
    AImage_getHeight(image, &h);

    int len = 0;
    uint8_t* data = nullptr;
    AImage_getPlaneData(image, 0, &data, &len);
    DIR *dir = opendir(dcim);
    if (dir) closedir(dir); else { LOGI("Invalid dcim path: %s", dcim) return; }

    struct timespec ts { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm localTime;
    localtime_r(&ts.tv_sec, &localTime);

    std::string file_name = dcim + std::string("/IMG_");
    std::string dash("-");
    file_name += std::to_string(localTime.tm_mon) +
                std::to_string(localTime.tm_mday) + dash +
                std::to_string(localTime.tm_hour) +
                std::to_string(localTime.tm_min) +
                std::to_string(localTime.tm_sec) + ".jpg";

    FILE *file = fopen(file_name.c_str(), "wb");
    if (file && data && len) {

        fwrite(data, 1, len, file);
        fclose(file);
    }
    LOGI("Got an image %s %d x %d", file_name.c_str(), w, h)
    AImage_delete(image);
}

static void imageCallback(void* preview_window, AImageReader* reader){

    int32_t format;
    media_status_t status = AImageReader_getFormat(reader, &format);
    ASSERT(status == AMEDIA_OK, "Failed to get format")

    if (format == AIMAGE_FORMAT_JPEG) {

        AImage* image = nullptr;
        status = AImageReader_acquireNextImage(reader, &image);
        ASSERT(status == AMEDIA_OK, "Image not available")

        std::thread write_file_handler(write_file, image);
        write_file_handler.detach();
    }
}   

static AImageReader_ImageListener jpg_listener {
            jpg_listener.context = nullptr,
            jpg_listener.onImageAvailable = imageCallback,
};

NDKCamera::NDKCamera(uint32_t id) {
    
    requests.resize(REQUESTS_COUNT);
    memset(requests.data(), 0, requests.size() * sizeof(RequestInfo));
    
    this->facing = (acamera_metadata_enum_acamera_lens_facing) id;
    LOGI("Createing camera facing= %d", id)
    // Load camera metadata
    using namespace NDKManager;
    for (int i=0; i<camera_list->numCameras; i++) {
        
        CALL(ACameraManager_getCameraCharacteristics(
            camera_manager.get(), camera_list->cameraIds[i], &metadata));

        ACameraMetadata_const_entry lens = {0};
        CALL(ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_FACING, &lens));
        auto entry_facing = (acamera_metadata_enum_acamera_lens_facing) lens.data.u8[0];

        if ((entry_facing == this->facing) && (status == ACAMERA_OK)) { camera_id = camera_list->cameraIds[i];  break; }
        ACameraMetadata_free(metadata); // do not remove!
    }
    calc_still_capture_size(AIMAGE_FORMAT_JPEG);
}

NDKCamera::~NDKCamera() {

    LOGI("Destroy camera %d", facing)
    
    start_preview(false);
    close_session();
    ogl::destroy();

    if (metadata) ACameraMetadata_free(metadata);
    metadata = nullptr;

    requests.resize(0);

    LOGI("Camera Destroyed")
}


void NDKCamera::create_session(void* win) noexcept {

    if (session_created) return;
    LOGI("Creating session")

    ANativeWindow* window = reinterpret_cast<ANativeWindow*>(win);
    ANativeWindow* jpg_window;

    CALL(ACameraManager_openCamera(NDKManager::camera_manager.get(), camera_id, &cameraDeviceCallbacks, &device))
    
    auto container = output_container { [this]() {

            ACaptureSessionOutputContainer* container{};
            CALL(ACaptureSessionOutputContainer_create(&container));
            return container;
        }(), ACaptureSessionOutputContainer_free
    };

    reader = image_reader { [&jpg_window, this]() {

            AImageReader* reader {};
            jpg_listener.context = this;
            AImageReader_new(still_cap_width, still_cap_height, AIMAGE_FORMAT_JPEG, 1, &reader);
            AImageReader_setImageListener(reader, &jpg_listener);
            AImageReader_getWindow(reader, &jpg_window);
            return reader;
        }(), AImageReader_delete
    };
    
    requests[PREVIEW_IDX].req_template = TEMPLATE_PREVIEW;
    requests[PREVIEW_IDX].window = {window, ANativeWindow_release};
    requests[STILL_CAPTURE_IDX].req_template = TEMPLATE_STILL_CAPTURE;
    requests[STILL_CAPTURE_IDX].window = {jpg_window, ANativeWindow_release};

    for (auto &req: requests) {
      
        ANativeWindow_acquire(req.window.get());

        req.output = session_output {  [&req, this] () {

                    ACaptureSessionOutput* output {};
                    CALL(ACaptureSessionOutput_create(req.window.get(), &output))
                    return output;
            }(), ACaptureSessionOutput_free
        };
        CALL(ACaptureSessionOutputContainer_add(container.get(), req.output.get()));

        req.target = output_target { [&req, this] () {

                    ACameraOutputTarget* target {};
                    CALL(ACameraOutputTarget_create(req.window.get(), &target))
                    return target;
            }(), ACameraOutputTarget_free
        };

        req.request = capture_request { [this] (ACameraDevice_request_template request_template) {

                ACaptureRequest* request {};
                CALL(ACameraDevice_createCaptureRequest(device, request_template, &request))
                return request;
            } (req.req_template), ACaptureRequest_free
        };
        CALL(ACaptureRequest_addTarget(req.request.get(), req.target.get()))
    }
    
    CALL(ACameraDevice_createCaptureSession(device, container.get(), &sessionStateCallbacks, &session))
    CALL(ACaptureSessionOutputContainer_remove(container.get(), requests[PREVIEW_IDX].output.get()))
    CALL(ACaptureSessionOutputContainer_remove(container.get(), requests[STILL_CAPTURE_IDX].output.get()))
    session_created = true; 
    LOGI("Preview session created, %d", status)
}

void NDKCamera::close_session() noexcept {

    if (session_created) {

        ACameraCaptureSession_close(session);
        CALL(ACameraDevice_close(device));
        device = nullptr;
        session_created = false;
        LOGI("Session closed")
    }
}

void NDKCamera::take_photo(const char* path) noexcept {

    if (preview_started) {

        dcim = path;
        ACaptureRequest* request = requests[STILL_CAPTURE_IDX].request.get();
        CALL(ACameraCaptureSession_capture(
            session, &captureCallbacks, 1, &request, &requests[STILL_CAPTURE_IDX].sequence))
    }
}

void NDKCamera::start_preview(bool start) noexcept {

    if (start) {

        if (preview_started) return;
        ACaptureRequest* request = requests[PREVIEW_IDX].request.get();
        CALL(ACameraCaptureSession_setRepeatingRequest(session, nullptr, 1, &request, nullptr))  
        preview_started = true; 
    } else {

        if (!preview_started) return;
        CALL(ACameraCaptureSession_stopRepeating(session))
        LOGI("Preview stopped")
        preview_started = false;
    }
}

void NDKCamera::calc_still_capture_size(AIMAGE_FORMATS format) {

    avalabale_stream_conf(format, [this] (int32_t width, int32_t height) {

            this->still_cap_width = std::max(this->still_cap_width, width);
            this->still_cap_height = std::max(this->still_cap_height, height);
    });
    LOGI("Still capture size, %d x %d", this->still_cap_width, this->still_cap_height)
}

void NDKCamera::avalabale_stream_conf(AIMAGE_FORMATS format, 
        std::function<void(int32_t width, int32_t height)> callback) {

    ACameraMetadata_const_entry entry = {0};
    // format, width, height, input?
    CALL(ACameraMetadata_getConstEntry(metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry));

    for (int i=0; i<entry.count; i+=4) {

        int32_t input = entry.data.i32[i+3];
        if (input) continue;  // intresting output
        
        if (entry.data.i32[i+0] == format) {

            callback(entry.data.i32[i+1], entry.data.i32[i+2]);
            LOGI("AVALABLE CONFIGURATION, %d x %d", entry.data.i32[i+1], entry.data.i32[i+2])
        }
    }     
}
    // Looking for compatible resolution with minimum differenc in greater side
void NDKCamera::calc_compatible_preview_size(int32_t width, int32_t height, int32_t out_compatible_res[2]) noexcept {

    screen_width = width; screen_height = height;
    int32_t window_square = width * height;
    int32_t min = 0;

    avalabale_stream_conf(AIMAGE_FORMAT_YUV_420_888, 
        [window_square, &min, out_compatible_res] (int32_t w, int32_t h) {

            int32_t diff = w * h - window_square;
            if ((min == 0) || ((diff > 0) && (diff < min))) {

                  min = diff;
                  out_compatible_res[0] = w;
                  out_compatible_res[1] = h;
            }
    });
    LOGI("Compatible preview size, %d x %d", out_compatible_res[0], out_compatible_res[1])
}

int32_t NDKCamera::sensor_orientation() noexcept {

    ACameraMetadata_const_entry entry = {0};
    CALL(ACameraMetadata_getConstEntry(metadata, ACAMERA_SENSOR_ORIENTATION, &entry))
    LOGI("ACAMERA_SENSOR_ORIENTATION, %d", entry.data.i32[0])
    return entry.data.i32[0];
}

void NDKCamera::init_surface(int32_t texture_id) noexcept {

    ogl::Properties props {screen_width, screen_height, 0, 0, sensor_orientation(), texture_id};
    ogl::init_surface(props);
}

void NDKCamera::draw_frame() noexcept {

    ogl::draw_frame();
}

void NDKCamera::next_shader() noexcept {
    ogl::next_filter();
}
