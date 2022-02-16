package com.home.camera.fragments

import android.annotation.SuppressLint
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.util.Log
import android.view.*
import androidx.fragment.app.Fragment
import com.home.camera.CamRenderer
import com.home.camera.databinding.FragmentCameraGlBinding

private const val logTag = "CameraFragmentGL"

class CameraFragmentGL:Fragment(), View.OnTouchListener {

    private lateinit var  binding: FragmentCameraGlBinding
    private lateinit var surfaceView: GLSurfaceView
    private lateinit var camRenderer: CamRenderer

    @SuppressLint("ClickableViewAccessibility")
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        binding = FragmentCameraGlBinding.inflate(inflater,container, false)
        surfaceView = binding.surfaceView
        surfaceView.setEGLContextClientVersion(3)
        camRenderer = CamRenderer(surfaceView)
        surfaceView.setRenderer(camRenderer)
        surfaceView.setOnTouchListener(this)
        surfaceView.renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
        return binding.root
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.d(logTag, "onDestroy")
        camRenderer.destroy()
    }

    override fun onPause() {
        super.onPause()
        Log.d(logTag, "omPause")
        surfaceView.isEnabled = false
    }

    override fun onTouch(p0: View?, event: MotionEvent?): Boolean {

        return when (event?.action) {
            MotionEvent.ACTION_DOWN -> {
                Log.d(logTag, "MotionEvent.ACTION_DOWN")
                true
            }
            MotionEvent.ACTION_UP -> {
                Log.d(logTag, "MotionEvent.ACTION_UP")
                p0?.performClick()
                camRenderer.applyNextShader()
                true
            }
            else -> {
                Log.d(logTag, "MotionEvent.ELSE")
                false
            }
        }
    }
    //TODO: Link button with lib
    // begin with net
}
