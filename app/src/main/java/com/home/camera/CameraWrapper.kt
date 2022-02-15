package com.home.camera

import android.util.Log
import android.view.Surface

class CameraWrapper {
    private val logTag = "CameraWrapper"

    private external fun greeting(name: String): String
    external fun create(cameraFacing: String):Long
    external fun delete(camera: Long)
    external fun compatibleResolution(camera: Long, width: Int, height: Int):IntArray
    external fun onPreviewSurfaceCreated(camera: Long, surface: Surface, texture_id: Int = 0)
    external fun onPreviewSurfaceDestroyed(camera: Long, surface: Surface?)
    external fun onDrawFrame(camera: Long, tex_mat: FloatArray)
    external fun nextShader(camera: Long)

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