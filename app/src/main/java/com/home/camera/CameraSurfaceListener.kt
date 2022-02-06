package com.home.camera

import android.graphics.SurfaceTexture
import android.util.Log
import android.view.TextureView

class CameraSurfaceListener : TextureView.SurfaceTextureListener, SurfaceTexture.OnFrameAvailableListener {

    private val tag = "CameraSurfaceListener"
    override fun onSurfaceTextureAvailable(surfaceTexture: SurfaceTexture, width: Int, height: Int) {
        Log.i(tag, "onSurfaceTextureAvailable $width x $height")
    }

    override fun onSurfaceTextureSizeChanged(p0: SurfaceTexture,  width: Int, height: Int) {
        Log.i(tag, "onSurfaceTextureSizeChanged $width x $height")
    }

    override fun onSurfaceTextureDestroyed(p0: SurfaceTexture): Boolean {
        Log.i(tag, "onSurfaceTextureDestroyed")
        return false
    }

    override fun onSurfaceTextureUpdated(p0: SurfaceTexture) {
        Log.i(tag, "onSurfaceTextureUpdated")
    }

    override fun onFrameAvailable(p0: SurfaceTexture?) {
        Log.i(tag, "onFrameAvailable")
    }
}