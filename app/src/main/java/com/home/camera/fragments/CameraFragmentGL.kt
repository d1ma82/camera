package com.home.camera.fragments

import android.annotation.SuppressLint
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.util.Log
import android.view.*
import androidx.fragment.app.Fragment
import com.home.camera.CamRenderer

private const val logTag = "CameraFragmentGL"

class CameraFragmentGL:Fragment(), View.OnTouchListener {

    private lateinit var surfaceView: GLSurfaceView
    private lateinit var camRenderer: CamRenderer

    @SuppressLint("ClickableViewAccessibility")
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        surfaceView = GLSurfaceView(requireContext())
        surfaceView.setEGLContextClientVersion(3)
        camRenderer = CamRenderer(surfaceView)
        surfaceView.setRenderer(camRenderer)
        surfaceView.setOnTouchListener(this)
        surfaceView.renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
        return surfaceView
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
    //TODO: Create button to made pictures
    // save picture to gallery
    // begin with neural net
}
