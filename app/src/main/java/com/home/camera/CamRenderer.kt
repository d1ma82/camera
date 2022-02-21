package com.home.camera

import android.graphics.SurfaceTexture
import android.opengl.GLES11Ext
import android.opengl.GLES30
import android.opengl.GLSurfaceView
import android.util.Log
import android.view.Surface
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10


class CamRenderer(private val view: GLSurfaceView, private val dcim: String):
        GLSurfaceView.Renderer, SurfaceTexture.OnFrameAvailableListener {

    private val logTag = "CamRenderer"
    private var cameraHandle:Long = 0
    private val textures = IntArray(1)
    private lateinit var surfaceTexture: SurfaceTexture
    private lateinit var surface: Surface
   // private var arr = FloatArray(16)
    private val cameraWrapper = CameraWrapper()
    private var selectedCamera:Int = 0

    fun destroy() {

        Log.d(logTag, "onDestroy")

        cameraWrapper.delete(cameraHandle)
        cameraHandle = 0
    }

    fun applyNextShader() {
        Log.d(logTag, "applyNextShader")
        cameraWrapper.nextShader(cameraHandle)
    }

    fun flipCamera() {

        Log.d(logTag, "flipCamera")
        selectedCamera++
        if (selectedCamera >= 2) selectedCamera = 0
        cameraWrapper.selectCamera(cameraHandle, selectedCamera)
        onSurfaceChanged(null, view.width, view.height)
    }

    fun takePhoto() {

        Log.d(logTag, "takePhoto")
        cameraWrapper.takePhoto(cameraHandle)
    }

    override fun onSurfaceCreated(p0: GL10?, p1: EGLConfig?) {
        Log.d(logTag, "onSurfaceCreated")

            GLES30.glGenTextures(1, textures, 0)
            GLES30.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textures[0])
            cameraHandle = cameraWrapper.create("android", dcim)
            surfaceTexture = SurfaceTexture(textures[0])
            surfaceTexture.setOnFrameAvailableListener(this)
            surface = Surface(surfaceTexture)
    }

    override fun onSurfaceChanged(p0: GL10?, width: Int, height: Int) {
        Log.d(logTag, "onSurfaceChanged $width x $height; handle = $cameraHandle")

        if (!view.isEnabled) {
            view.isEnabled = true
            return
        }
        val compResolution = cameraWrapper.compatibleResolution(cameraHandle, width, height)
        surfaceTexture.setDefaultBufferSize(compResolution[0], compResolution[1])
        cameraWrapper.onPreviewSurfaceCreated(cameraHandle, surface, textures[0])
    }

    override fun onDrawFrame(p0: GL10?) {

        if (!view.isEnabled) return
        cameraWrapper.onDrawFrame(cameraHandle)
    }

    override fun onFrameAvailable(p0: SurfaceTexture?) {

        if (!view.isEnabled) return
        view.queueEvent{

            p0?.updateTexImage()
            //p0?.getTransformMatrix(arr)
           // Log.d(logTag, arr.contentToString())
            view.requestRender()
        }
    }
}
