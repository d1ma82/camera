package com.home.camera.fragments

import android.graphics.SurfaceTexture
import android.opengl.GLES11Ext
import android.opengl.GLES20
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.Surface
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.home.camera.CameraWrapper
import com.home.camera.databinding.FragmentCameraGlBinding
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class CameraFragmentGL:Fragment(), SurfaceTexture.OnFrameAvailableListener, GLSurfaceView.Renderer {

    private val logTag = "CameraFragmentGL"
    private var cameraHandle:Long = 0
    private val texMatrix = FloatArray(16)
    private val cameraWrapper = CameraWrapper()
    private lateinit var binding: FragmentCameraGlBinding
    private lateinit var surfaceTexture: SurfaceTexture
    private lateinit var surface: Surface

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        binding = FragmentCameraGlBinding.inflate(inflater,container, false)
        return binding.root
    }

    override fun onStart() {
        super.onStart()
        Log.d(logTag, "onStart ${binding.previewSurfaceGl.width} x ${binding.previewSurfaceGl.height}")
        binding.previewSurfaceGl.setEGLContextClientVersion(2)
        binding.previewSurfaceGl.setRenderer(this)
        binding.previewSurfaceGl.renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.d(logTag, "onDestroy")
        cameraWrapper.onPreviewSurfaceDestroyed(cameraHandle, surface)
        cameraWrapper.delete(cameraHandle)
        cameraHandle = 0
    }

    override fun onSurfaceCreated(p0: GL10?, p1: EGLConfig?) {

        val width = binding.previewSurfaceGl.width
        val height = binding.previewSurfaceGl.height
        Log.d(logTag, "onSurfaceCreated $width x $height")

        val textures = IntArray(1)
        GLES20.glGenTextures(1, textures, 0)
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textures[0])
        surfaceTexture = SurfaceTexture(textures[0])
        surfaceTexture.setOnFrameAvailableListener(this)
        cameraHandle = cameraWrapper.create("back")
        val compatibleResolution = cameraWrapper.compatibleResolution(cameraHandle, width, height)
        surfaceTexture.setDefaultBufferSize(compatibleResolution[0], compatibleResolution[1])
        surface = Surface(surfaceTexture)
        cameraWrapper.onPreviewSurfaceCreated(cameraHandle, surface, textures[0])
    }

    override fun onSurfaceChanged(p0: GL10?, w: Int, h: Int) {
        Log.d(logTag, "onSurfaceChanged $w x $h")
    }

    override fun onDrawFrame(p0: GL10?) {

        Log.d(logTag, "onDrawFrame")
        cameraWrapper.onDrawFrame(cameraHandle, texMatrix)
    }

    override fun onFrameAvailable(p0: SurfaceTexture?) {

        binding.previewSurfaceGl.queueEvent{
            p0?.updateTexImage()
            p0?.getTransformMatrix(texMatrix)
           // Log.d(logTag, texMatrix.contentToString())
            binding.previewSurfaceGl.requestRender()
        }
    }

    /*0.0, -1.0, 0.0, 0.0,
    -1.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    1.0, 1.0, 0.0, 1.0*/
}