package com.home.camera.fragments

import android.opengl.GLSurfaceView
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.home.camera.CameraRender
import com.home.camera.databinding.FragmentCameraGlBinding

class CameraFragmentGL:Fragment() {

    //private val logTag = "CameraFragmentGL"
    private lateinit var binding: FragmentCameraGlBinding
    private val renderer= CameraRender()

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
        binding.previewSurfaceGl.setEGLContextClientVersion(3)
        binding.previewSurfaceGl.setRenderer(renderer)
        binding.previewSurfaceGl.renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
    }
}