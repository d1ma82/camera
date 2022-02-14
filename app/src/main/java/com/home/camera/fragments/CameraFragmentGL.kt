package com.home.camera.fragments

import android.opengl.GLSurfaceView
import android.os.Bundle
import android.util.Log
import android.view.*
import androidx.fragment.app.Fragment
import com.home.camera.CamRenderer

class CameraFragmentGL:Fragment() {

    private val logTag = "CameraFragmentGL"
    private lateinit var surfaceView: GLSurfaceView
    private lateinit var camRenderer: CamRenderer

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        surfaceView = GLSurfaceView(requireContext())
        surfaceView.setEGLContextClientVersion(3)
        camRenderer = CamRenderer(surfaceView)
        surfaceView.setRenderer(camRenderer)
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
    }
}