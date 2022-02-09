#ifndef _CAMERA_MANAGER_H
#define _CAMERA_MANAGER_H

#include <camera/NdkCameraManager.h>
#include <functional>
#include <vector>

using session_output = std::unique_ptr<ACaptureSessionOutput, std::function<void(ACaptureSessionOutput*)>>;
using output_target = std::unique_ptr<ACameraOutputTarget, std::function<void(ACameraOutputTarget*)>>;
using capture_request = std::unique_ptr<ACaptureRequest, std::function<void(ACaptureRequest*)>>;
using native_window = std::unique_ptr<ANativeWindow, std::function<void(ANativeWindow*)>>;

struct RequestInfo {
    native_window window = nullptr;
    session_output output = nullptr;
    output_target target = nullptr;
    capture_request request = nullptr;
    ACameraDevice_request_template req_template;
    int session_sequence;
};

class NDKCamera {
private:
    camera_status_t status = ACAMERA_OK;

    ACameraManager* manager = nullptr;
    ACameraIdList* id_list = nullptr;
    ACameraDevice* device = nullptr;
    ACameraMetadata* metadata = nullptr;
    ACameraCaptureSession* session = nullptr;
    ACaptureSessionOutputContainer* container = nullptr;
    
    std::vector<RequestInfo*> requests;
   
    const  char* cam_id = nullptr;
    uint32_t facing = ACAMERA_LENS_FACING_BACK;
    uint32_t orientation = 0;

    void calc_compatible_preview_size(int32_t requested_width, int32_t requested_height) noexcept;
public:
    NDKCamera(const char* facing, int32_t requested_width, int32_t requested_height);
   // copy-move disabled
    NDKCamera(const NDKCamera&) = delete;
    NDKCamera(NDKCamera&&) = delete;
    NDKCamera& operator=(const NDKCamera&) = delete;
    NDKCamera& operator=(NDKCamera&&) = delete;
    ~NDKCamera();  

    void select_camera(const char* facing) noexcept;
    void create_session(ANativeWindow* window) noexcept;
    void start_preview(bool start) noexcept;
    void compatible_preview_size(int32_t compatible_size[2]) noexcept;
  //  void repeat(uint32_t tex_id, jobject preview) noexcept;
   // static void on_draw(float texMat[16]) noexcept;
   // static void surface_changes(int32_t width, int32_t height) noexcept;
};

#endif