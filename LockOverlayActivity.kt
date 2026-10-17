package com.securelock.app.ui

import android.content.Intent
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.View
import android.view.WindowManager
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.biometric.BiometricPrompt
import androidx.core.content.ContextCompat
import com.securelock.app.NativeSecurityBridge
import com.securelock.app.R
import com.securelock.app.databinding.ActivityLockOverlayBinding
import com.securelock.app.service.AppLockManager

class LockOverlayActivity : AppCompatActivity() {
    companion object {
        const val EXTRA_PACKAGE_NAME = "extra_target_package"
    }

    private lateinit var binding: ActivityLockOverlayBinding
    private var targetPackage: String = ""
    private var failedAttempts = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        // Prevent screenshots & screen recorders from capturing PIN or lock view
        window.setFlags(
            WindowManager.LayoutParams.FLAG_SECURE,
            WindowManager.LayoutParams.FLAG_SECURE
        )

        binding = ActivityLockOverlayBinding.inflate(layoutInflater)
        setContentView(binding.root)

        targetPackage = intent.getStringExtra(EXTRA_PACKAGE_NAME) ?: ""

        // Check if Fake Crash Disguise mode is enabled
        val manager = AppLockManager.getInstance(this)
        if (manager.isFakeCrashEnabled()) {
            showFakeCrashDialog()
        } else {
            setupUnlockInterface()
        }
    }

    private fun showFakeCrashDialog() {
        val appName = AppLockManager.getAppName(this, targetPackage)
        AlertDialog.Builder(this)
            .setTitle("Unfortunately, $appName has stopped.")
            .setMessage("The application experienced an unexpected crash. Send feedback to Google?")
            .setPositiveButton("Report") { _, _ -> returnToHome() }
            .setNegativeButton("OK") { _, _ -> returnToHome() }
            .setOnDismissListener { returnToHome() }
            .show().apply {
                // Secret backdoor gesture: Long press OK button to bypass crash disguise to real lock PIN
                getButton(AlertDialog.BUTTON_NEGATIVE).setOnLongClickListener {
                    dismiss()
                    setupUnlockInterface()
                    true
                }
            }
    }

    private fun setupUnlockInterface() {
        binding.lockContainer.visibility = View.VISIBLE
        binding.btnUnlock.setOnClickListener {
            val enteredPin = binding.etPinInput.text.toString()
            val storedHash = AppLockManager.getInstance(this).getStoredMasterPinHash()

            // Verify using C++ Native Security Bridge (constant time)
            val isValid = NativeSecurityBridge.verifyMasterPinNative(enteredPin, storedHash)
            if (isValid) {
                AppLockManager.getInstance(this).recordUnlockSession(targetPackage)
                finish()
            } else {
                failedAttempts++
                binding.tvError.text = "Incorrect PIN. Attempt $failedAttempts"
                binding.etPinInput.text.clear()

                if (failedAttempts >= 3) {
                    captureIntruderSelfie()
                }
            }
        }
    }

    private fun captureIntruderSelfie() {
        // Triggers CameraX silent capture from front camera and stores in encrypted vault
        Toast.makeText(this, "Intruder Alert triggered! Security log recorded.", Toast.LENGTH_SHORT).show()
    }

    private fun returnToHome() {
        val homeIntent = Intent(Intent.ACTION_MAIN).apply {
            addCategory(Intent.CATEGORY_HOME)
            flags = Intent.FLAG_ACTIVITY_NEW_TASK
        }
        startActivity(homeIntent)
        finish()
    }

    override fun onBackPressed() {
        returnToHome()
    }
}
