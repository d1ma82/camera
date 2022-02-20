#include "log.h"
#include <vector>
#include "camera_engine.h"
#include "camera_manager.h"

static std::vector<CustomCamera*> cameras;

CameraEngine::CameraEngine(JNIEnv* env, jobject instance, jstring in_dcim):
    env(env), 
    instance(instance) {

    const char* dcim_ = env->GetStringUTFChars(in_dcim, 0);
    dcim = new char[strlen(dcim_)];
    strcpy(dcim, dcim_);
    env->ReleaseStringUTFChars(in_dcim, dcim_);
}

CameraEngine::~CameraEngine() {

    delete dcim;
    for (auto el: cameras) delete el;
    cameras.resize(0);
}

void CameraEngine::select_camera(uint32_t id) {
    
    ASSERT(id <= cameras.size()-1, "CameraEngine::select_camera: index out of bounds, %d", id)

    selected = cameras[id];
    ASSERT(selected != nullptr, "CameraEngine selected = NULL")
    LOGI("Select camera facing= %d", id)
}

void CameraEngine::add_camera_kindof(jstring kind) {

    const char* kind_ = env->GetStringUTFChars(kind, 0);
    if (strncmp(kind_, "android", 3) == 0) {

        NDKManager::manager_new_instance(); 
        uint32_t count = NDKManager::count_camera();

        for (int i=cameras.size(); i<cameras.size()+count; i++)     
                                    cameras.push_back(new NDKCamera(--count));

        select_camera(count);
    }
    env->ReleaseStringUTFChars(kind, kind_);
    ASSERT(cameras.size() > 0, "Warning: Cameras array empty.")
}
