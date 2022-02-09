package com.home.camera

import android.util.Log
import android.view.Surface

class CameraWrapper() {
    private val LOGTAG = "CameraWrapper"

    external fun greeting(name: String): String
    external fun create(cameraFacing: String, width: Int, height:Int):Long
    external fun delete(camera: Long)
    external fun compatiblePreviewResolution(camera: Long):IntArray
    external fun onPreviewSurfaceCreated(camera: Long, surface: Surface)
    external fun onPreviewSurfaceDestroyed(camera: Long, surface: Surface)

    init {
        val greet = greeting("Kotlin")
        Log.d(LOGTAG, greet)
    }

    companion object {
        init {
            System.loadLibrary("camera-lib")
        }
    }
}