#ifndef _CAMERA_MANAGER_H
#define _CAMERA_MANAGER_H

#include <camera/NdkCameraManager.h>
#include <media/NdkImageReader.h>
#include <functional>
#include <vector>


using output_target = std::unique_ptr<ACameraOutputTarget, std::function<void(ACameraOutputTarget*)>>;
using capture_request = std::unique_ptr<ACaptureRequest, std::function<void(ACaptureRequest*)>>;
using image_reader = std::unique_ptr<AImageReader, std::function<void(AImageReader*)>>;
using native_window = std::unique_ptr<ANativeWindow, std::function<void(ANativeWindow*)>>;
using session_output = std::unique_ptr<ACaptureSessionOutput, std::function<void(ACaptureSessionOutput*)>>;


struct RequestInfo {
    //native_window window = nullptr;
    native_window window = nullptr;
    session_output output = nullptr;
    output_target target = nullptr;
    capture_request request = nullptr;
    ACameraDevice_request_template req_template;
    int sequence;
};

enum RequestIndex{PREVIEW_IDX, PHOTO_IDX, REQUESTS_COUNT};

const AIMAGE_FORMATS STILL_CAPTURE_FORMAT = AIMAGE_FORMAT_JPEG;

class NDKCamera {
private:
    camera_status_t status = ACAMERA_OK;

    ACameraManager* manager = nullptr;
    ACameraIdList* id_list = nullptr;
    ACameraDevice* device = nullptr;
    ACameraMetadata* metadata = nullptr;
    ACameraCaptureSession* session = nullptr;
    ACaptureSessionOutputContainer* container = nullptr;
    image_reader reader = nullptr;
    
    std::vector<RequestInfo> requests;
   
    const  char* cam_id = nullptr;
    uint32_t facing = ACAMERA_LENS_FACING_BACK;
    
    void avalabale_stream_conf(AIMAGE_FORMATS format, 
        std::function<void(int32_t width, int32_t height)> callback);
    void still_capture_size(AIMAGE_FORMATS format);
public:
    NDKCamera(const char* facing);
   // copy-move disabled
    NDKCamera(const NDKCamera&) = delete;
    NDKCamera(NDKCamera&&) = delete;
    NDKCamera& operator=(const NDKCamera&) = delete;
    NDKCamera& operator=(NDKCamera&&) = delete;
    ~NDKCamera();  

    int32_t sensor_orientation() noexcept;
    void select_camera(const char* facing) noexcept;
    void create_session(ANativeWindow* window) noexcept;
    void start_preview(bool start) noexcept;
    void take_photo(const char* path) noexcept;
    void calc_compatible_preview_size(int32_t width, int32_t height, int32_t out_compatible_res[2]) noexcept;
    void init_surface(int32_t texture_id) noexcept;
    void next_shader() noexcept;
    void draw_frame() noexcept;
};

#endif