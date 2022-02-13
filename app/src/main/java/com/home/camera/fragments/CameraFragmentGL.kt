package com.home.camera.fragments

import android.content.Context
import android.os.Bundle
import android.util.DisplayMetrics
import android.util.Log
import android.view.*
import androidx.fragment.app.Fragment
import com.home.camera.databinding.FragmentCameraGlBinding


class CameraFragmentGL:Fragment() {

    private val logTag = "CameraFragmentGL"
    private lateinit var binding : FragmentCameraGlBinding

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
        val wm = requireContext().getSystemService(Context.WINDOW_SERVICE) as WindowManager
        val metrics = DisplayMetrics()
        wm.defaultDisplay.getMetrics(metrics)
        binding.previewSurface.onStart(metrics.widthPixels, metrics.heightPixels)
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.d(logTag, "onDestroy")
        binding.previewSurface.onDestroy()
    }

    override fun onPause() {
        super.onPause()
        Log.d(logTag, "omPause")
        binding.previewSurface.onDestroy()
    }
}