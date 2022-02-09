package com.home.camera

import android.graphics.SurfaceTexture
import android.util.Log
import android.view.Surface
import android.view.TextureView

class CameraRender: TextureView.SurfaceTextureListener {


    override fun onSurfaceTextureAvailable(surfaceTexture: SurfaceTexture, width: Int, height: Int) {

    }

    override fun onSurfaceTextureSizeChanged(p0: SurfaceTexture, p1: Int, p2: Int) {

    }

    override fun onSurfaceTextureDestroyed(p0: SurfaceTexture): Boolean {
       return false
    }

    override fun onSurfaceTextureUpdated(p0: SurfaceTexture) {

    }
}