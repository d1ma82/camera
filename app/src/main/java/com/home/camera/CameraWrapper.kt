package com.home.camera

class CameraWrapper {
    external fun greeting(name: String): String

    companion object {
        init {
            System.loadLibrary("camera-lib")
        }
    }
}