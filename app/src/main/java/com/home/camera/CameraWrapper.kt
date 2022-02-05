package com.home.camera

class CameraWrapper {
    external fun greeting(name: String): String
    external fun select(cameraType: String, cameraId: String)

    companion object {
        init {
            System.loadLibrary("camera-lib")
        }
    }
}