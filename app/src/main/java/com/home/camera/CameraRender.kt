package com.home.camera

import android.graphics.SurfaceTexture
import android.opengl.GLES11Ext.GL_TEXTURE_EXTERNAL_OES
import android.opengl.GLES30
import android.opengl.GLSurfaceView
import android.util.Log
import android.view.Surface
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class CameraRender: GLSurfaceView.Renderer  {

    private val logTag = "CameraRender"
    private var cameraHandle:Long = 0
    private var repeating: Boolean = false
    private val cameraWrapper = CameraWrapper()
    private lateinit var surfaceTexture: SurfaceTexture
    private lateinit var compatibleResolution: IntArray
    private val texMatrix = FloatArray(16)
    @Volatile var frameAvailable: Boolean = false
    private val lock = Object()

    override fun onSurfaceCreated(p0: GL10?, p1: EGLConfig?) {

        Log.d(logTag, "onSurfaceCreated")

        cameraHandle = cameraWrapper.create("back")

        val textures = IntArray(1)
        GLES30.glGenTextures(1, textures, 0)
        GLES30.glBindTexture(GL_TEXTURE_EXTERNAL_OES, textures[0])

        surfaceTexture = SurfaceTexture(textures[0])

        surfaceTexture.setOnFrameAvailableListener {
            synchronized(lock) {frameAvailable = true}
        }
        cameraWrapper.onSurfaceCreated(cameraHandle, textures[0])
    }

    override fun onSurfaceChanged(p0: GL10?, w: Int, h: Int) {

        if (repeating) return

        Log.d(logTag, "onSurfaceChanged $w x $h")

        compatibleResolution = cameraWrapper.compatibleResolution(cameraHandle, w, h)
        surfaceTexture.setDefaultBufferSize(compatibleResolution[0], compatibleResolution[1])
        val surface = Surface(surfaceTexture)
        cameraWrapper.onSurfaceChanged(cameraHandle, surface)
        repeating = true
    }

    override fun onDrawFrame(p0: GL10?) {

        Log.d(logTag, "onDrawFrame...updating")
        synchronized(lock) {
            if (frameAvailable) {
                surfaceTexture.updateTexImage()
                surfaceTexture.getTransformMatrix(texMatrix)
                frameAvailable = false
            }
        }
        cameraWrapper.onDrawFrame(cameraHandle, compatibleResolution[0], compatibleResolution[1], texMatrix)
    }
}