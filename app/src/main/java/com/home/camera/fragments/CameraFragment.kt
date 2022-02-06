package com.home.camera.fragments

import android.graphics.SurfaceTexture
import android.os.Bundle
import android.util.Log
import android.view.*
import android.widget.Toast
import androidx.fragment.app.Fragment
import com.home.camera.CameraSurfaceListener
import com.home.camera.CameraWrapper
import com.home.camera.databinding.FragmentCameraBinding

class CameraFragment: Fragment(), TextureView.SurfaceTextureListener {

    private val t = "CameraFragment"

    private lateinit var binding: FragmentCameraBinding
    private val cameraWrapper = CameraWrapper()
    private lateinit var bestResolution:IntArray
    private lateinit var cameraTextureView : TextureView
    private val cameraSurfaceTexture = SurfaceTexture(666) // TODO: generate via OpenGL
    private val cameraSurfaceListener = CameraSurfaceListener()

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        binding = FragmentCameraBinding.inflate(inflater,container, false)
        binding.previewSurface.surfaceTextureListener = this
        cameraSurfaceTexture.setOnFrameAvailableListener(cameraSurfaceListener)
        cameraTextureView = TextureView(requireContext())
        cameraTextureView.setSurfaceTexture(cameraSurfaceTexture)
        cameraTextureView.surfaceTextureListener = cameraSurfaceListener
        Log.i(t,"cameraTextureView.isAvailable ${cameraTextureView.isAvailable}")
        return binding.root
    }

    override fun onStart() {
        super.onStart()
        cameraWrapper.select("android", "back")

        val str = cameraWrapper.greeting("kotlin")
        Toast.makeText(context, str, Toast.LENGTH_LONG).show()
    }

    override fun onSurfaceTextureAvailable(surfaceTexture: SurfaceTexture, width: Int, height: Int) {
        Log.i(t, "onSurfaceTextureAvailable $width x $height")
        surfaceTexture.setDefaultBufferSize(width, height)
        bestResolution = cameraWrapper.bestPreviewResolution(binding.previewSurface.width, binding.previewSurface.height)
        cameraSurfaceTexture.setDefaultBufferSize(bestResolution[0], bestResolution[1])
        cameraWrapper.repeat(Surface(cameraSurfaceTexture))
    }

    override fun onSurfaceTextureSizeChanged(p0: SurfaceTexture, width: Int, height: Int) {
        Log.i(t, "onSurfaceTextureSizeChanged $width x $height")
    }

    override fun onSurfaceTextureDestroyed(p0: SurfaceTexture): Boolean {
        Log.i(t, "onSurfaceTextureDestroyed")
        return false
    }

    override fun onSurfaceTextureUpdated(p0: SurfaceTexture) {
        Log.i(t, "onSurfaceTextureUpdated")
    }
}