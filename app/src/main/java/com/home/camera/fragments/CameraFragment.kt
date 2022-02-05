package com.home.camera.fragments

import android.os.Bundle
import android.view.LayoutInflater
import android.view.TextureView
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import com.home.camera.CameraWrapper
import com.home.camera.databinding.FragmentCameraBinding

class CameraFragment: Fragment() {

    private lateinit var binding: FragmentCameraBinding
    private lateinit var cameraSurface: TextureView
    private val cameraWrapper = CameraWrapper()

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        binding = FragmentCameraBinding.inflate(inflater,container, false)
        return binding.root
    }

    override fun onStart() {
        super.onStart()
        val str = cameraWrapper.greeting("kotlin")
        Toast.makeText(context, str, Toast.LENGTH_LONG).show()
    }
}