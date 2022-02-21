package com.home.camera

import android.os.Bundle
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.home.camera.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private var pressedTime: Long = 0L
    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
    }

    override fun onResume() {
        super.onResume()
        // Before setting full screen flags, we must wait a bit to let UI settle; otherwise, we may
        // be trying to set app to immersive mode before it's ready and the flags do not stick
        binding.fragmentContainer.postDelayed({
            binding.fragmentContainer.systemUiVisibility =
                        FLAGS_FULLSCREEN}, IMMERSIVE_FLAG_TIMEOUT)
    }

    override fun onBackPressed() {

        if (pressedTime + 2000 > System.currentTimeMillis()) {
            finish()
        } else {
            Toast.makeText(baseContext, "Press back again to exit", Toast.LENGTH_LONG).show()
        }
        pressedTime = System.currentTimeMillis()
    }

    companion object {

        /** Combination of all flags required to put activity into immersive mode */
        const val FLAGS_FULLSCREEN=
            View.SYSTEM_UI_FLAG_LOW_PROFILE or
                    View.SYSTEM_UI_FLAG_FULLSCREEN or
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE or
                    View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY

        private const val IMMERSIVE_FLAG_TIMEOUT = 100L
    }
}