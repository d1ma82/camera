package com.home.camera

import android.util.Log
import android.view.Surface

class CameraWrapper {
    private val logTag = "CameraWrapper"

    private external fun greeting(name: String): String
    external fun create(cameraFacing: String, dcim: String):Long
    external fun delete(camera: Long)
    external fun compatibleResolution(camera: Long, width: Int, height: Int):IntArray
    external fun onPreviewSurfaceCreated(camera: Long, surface: Surface, texture_id: Int = 0)
    external fun onPreviewSurfaceDestroyed(camera: Long, surface: Surface?)
    external fun onDrawFrame(camera: Long)
    external fun nextShader(camera: Long)
    external fun takePhoto(camera: Long)

    init {
        val greet = greeting("Kotlin")
        Log.d(logTag, greet)
    }

    companion object {
        init {
            System.loadLibrary("camera-lib")
        }
    }
}