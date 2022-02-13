package com.home.camera

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import android.util.Log

class CamSurfaceView(context: Context?, attrs: AttributeSet) : GLSurfaceView(context, attrs){

    private val logTag = "CamSurfaceView"
    private var started: Boolean = false
    private lateinit var camRenderer:CamRenderer

    fun onStart(width: Int, height: Int) {

        Log.d(logTag, "onStart $width x $height")
        if (started) return

        camRenderer = CamRenderer(width, height, this)
        setRenderer(camRenderer)
        renderMode = RENDERMODE_WHEN_DIRTY
        started = true
    }

    fun onDestroy() {
        camRenderer.destroy()
    }

    init {
        setEGLContextClientVersion(3)
    }
}