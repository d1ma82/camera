package com.home.camera

import android.graphics.SurfaceTexture
import android.opengl.GLES11Ext
import android.opengl.GLES30
import android.opengl.GLSurfaceView
import android.util.Log
import android.view.Surface
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class CamRenderer(private val width: Int, private val height: Int, private val view: GLSurfaceView):
        GLSurfaceView.Renderer, SurfaceTexture.OnFrameAvailableListener {

    private val logTag = "CamRenderer"
    private var pause :Boolean = false
    private var cameraHandle:Long = 0
    private val texMatrix = FloatArray(16)
    private val textures = IntArray(1)
    private lateinit var surfaceTexture: SurfaceTexture
    private lateinit var surface: Surface
    private val cameraWrapper = CameraWrapper()

    fun destroy() {

        Log.d(logTag, "onDestroy")
        if (cameraHandle > 0) {
            pause = true
            cameraWrapper.onPreviewSurfaceDestroyed(cameraHandle, null)
            cameraWrapper.delete(cameraHandle)
            cameraHandle = 0
        }
    }

    override fun onSurfaceCreated(p0: GL10?, p1: EGLConfig?) {

        Log.d(logTag, "onSurfaceCreated")

        cameraHandle = cameraWrapper.create("back")

        GLES30.glGenTextures(1, textures, 0)
        GLES30.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textures[0])
        surfaceTexture = SurfaceTexture(textures[0])
        val compResolution = cameraWrapper.compatibleResolution(cameraHandle, width, height)

        surfaceTexture.setDefaultBufferSize(compResolution[0], compResolution[1])
        surfaceTexture.setOnFrameAvailableListener(this)
        surface = Surface(surfaceTexture)
        cameraWrapper.onPreviewSurfaceCreated(cameraHandle, surface, textures[0])
    }

    override fun onSurfaceChanged(p0: GL10?, width: Int, height: Int) {
        Log.d(logTag, "onSurfaceChanged $width x $height")
    }

    override fun onDrawFrame(p0: GL10?) {

        Log.d(logTag, "onDrawFrame")
        if (pause) return
        cameraWrapper.onDrawFrame(cameraHandle, texMatrix)
    }

    override fun onFrameAvailable(p0: SurfaceTexture?) {

        Log.d(logTag, "onFrameAvailable")

        if (pause) return

        view.queueEvent{

            p0?.updateTexImage()
            p0?.getTransformMatrix(texMatrix)
           // Matrix.translateM(texMatrix, 0, 0f, 1f, 0f)
            view.requestRender()
        }
    }
}