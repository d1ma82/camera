package com.home.camera

import android.view.Surface

class CameraWrapper {
    external fun greeting(name: String): String
    external fun select(cameraType: String, cameraId: String)
    external fun bestPreviewResolution(width: Int, height: Int): IntArray
    external fun repeat(surface: Surface)

    companion object {
        init {
            System.loadLibrary("camera-lib")
        }
    }
}