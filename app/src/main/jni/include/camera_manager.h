#ifndef _CAMERA_MANAGER_H
#define _CAMERA_MANAGER_H

#include "custom_camera.h"
#include <camera/NdkCameraManager.h>
#include <media/NdkImageReader.h>
#include <functional>

using output_container = std::unique_ptr<ACaptureSessionOutputContainer, std::function<void(ACaptureSessionOutputContainer*)>>;
using output_target = std::unique_ptr<ACameraOutputTarget, std::function<void(ACameraOutputTarget*)>>;
using capture_request = std::unique_ptr<ACaptureRequest, std::function<void(ACaptureRequest*)>>;
using image_reader = std::unique_ptr<AImageReader, std::function<void(AImageReader*)>>;
using native_window = std::unique_ptr<ANativeWindow, std::function<void(ANativeWindow*)>>;
using session_output = std::unique_ptr<ACaptureSessionOutput, std::function<void(ACaptureSessionOutput*)>>;
using manager = std::unique_ptr<ACameraManager, std::function<void(ACameraManager*)>>;
using id_list = std::unique_ptr<ACameraIdList, std::function<void(ACameraIdList*)>>;


namespace NDKManager {
    void manager_new_instance();
    uint32_t count_camera();
}

struct RequestInfo {
  
    native_window window = nullptr;
    session_output output = nullptr;
    output_target target = nullptr;
    capture_request request = nullptr;
    ACameraDevice_request_template req_template;
    int sequence;
};

enum RequestIndex{PREVIEW_IDX, STILL_CAPTURE_IDX, REQUESTS_COUNT};

const AIMAGE_FORMATS STILL_CAPTURE_FORMAT = AIMAGE_FORMAT_JPEG;

class NDKCamera: public CustomCamera {
public:
    NDKCamera(uint32_t id);
   // copy-move disabled
    NDKCamera(const NDKCamera&) = delete;
    NDKCamera(NDKCamera&&) = delete;
    NDKCamera& operator=(const NDKCamera&) = delete;
    NDKCamera& operator=(NDKCamera&&) = delete;
    ~NDKCamera();  

    void create_session(void* window) noexcept final;
    void close_session() noexcept final;
    void start_preview(bool start) noexcept final;
    void take_photo(const char* path) noexcept final;
    void calc_compatible_preview_size(int32_t width, int32_t height, int32_t out_compatible_res[2]) noexcept final;
    void init_surface(int32_t texture_id) noexcept final;
    void next_shader() noexcept final;
    void draw_frame() noexcept final;

private:
    camera_status_t status = ACAMERA_OK;
    ACameraDevice* device = nullptr;
    ACameraMetadata* metadata = nullptr;
    ACameraCaptureSession* session = nullptr;
    image_reader reader = nullptr;
    const char* camera_id = nullptr;

    acamera_metadata_enum_acamera_lens_facing facing = ACAMERA_LENS_FACING_BACK;
    int32_t screen_width=0, screen_height=0;
    int32_t still_cap_width=0, still_cap_height=0;
    bool session_created=false, preview_started=false;
    
    void avalabale_stream_conf(AIMAGE_FORMATS format, 
        std::function<void(int32_t width, int32_t height)> callback);
    void calc_still_capture_size(AIMAGE_FORMATS format);
    int32_t sensor_orientation() noexcept;
};

#endif