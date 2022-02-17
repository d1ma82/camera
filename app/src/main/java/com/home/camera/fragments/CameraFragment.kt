package com.home.camera.fragments

import android.graphics.SurfaceTexture
import android.os.Bundle
import android.util.Log
import android.view.*
import androidx.fragment.app.Fragment
import com.home.camera.CameraWrapper
import com.home.camera.databinding.FragmentCameraBinding

class CameraFragment: Fragment(), TextureView.SurfaceTextureListener{

    private val logTag = "CameraFragment"
    private var cameraHandle:Long = 0
    private lateinit var binding: FragmentCameraBinding
    private lateinit var surface: Surface
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
        val initCamera = Thread{
            binding.previewSurface.surfaceTextureListener = this
            if (binding.previewSurface.isAvailable) {
                onSurfaceTextureAvailable(binding.previewSurface.surfaceTexture!!,
                    binding.previewSurface.width, binding.previewSurface.height)
            }
        }
        initCamera.start()
    }

    override fun onSurfaceTextureAvailable(surfaceTexture: SurfaceTexture, width: Int, height: Int) {

        Log.d(logTag, "onSurfaceTextureAvailable $width x $height")
        cameraHandle = cameraWrapper.create("back", "")
        val compatibleResolution = cameraWrapper.compatibleResolution(cameraHandle, 1600, 1200)
        surfaceTexture.setDefaultBufferSize(compatibleResolution[0], compatibleResolution[1])
        surface = Surface(surfaceTexture)
        cameraWrapper.onPreviewSurfaceCreated(cameraHandle, surface)
    }

    override fun onSurfaceTextureSizeChanged(p0: SurfaceTexture, width: Int, height: Int) {
        Log.d(logTag, "onSurfaceTextureSizeChanged $width x $height")
    }

    override fun onSurfaceTextureDestroyed(p0: SurfaceTexture): Boolean {

        Log.d(logTag, "onSurfaceTextureDestroyed")
        cameraWrapper.onPreviewSurfaceDestroyed(cameraHandle, surface)
        cameraWrapper.delete(cameraHandle)
        cameraHandle = 0
        return true
    }

    override fun onSurfaceTextureUpdated(p0: SurfaceTexture) {
        Log.d(logTag, "onSurfaceTextureUpdated")
    }
}