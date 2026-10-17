package com.securelock.app.service

import android.accessibilityservice.AccessibilityService
import android.content.Intent
import android.util.Log
import android.view.accessibility.AccessibilityEvent
import com.securelock.app.ui.LockOverlayActivity

class AppLockAccessibilityService : AccessibilityService() {
    companion object {
        private const val TAG = "AppLockA11yService"
        var currentForegroundPackage: String? = null
    }

    override fun onAccessibilityEvent(event: AccessibilityEvent?) {
        if (event == null || event.eventType != AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED) {
            return
        }

        val pkgName = event.packageName?.toString() ?: return
        if (pkgName == packageName) return // Ignore self

        currentForegroundPackage = pkgName
        Log.d(TAG, "Foreground package changed to: $pkgName")

        val appLockManager = AppLockManager.getInstance(applicationContext)
        if (appLockManager.shouldLock(pkgName)) {
            val intent = Intent(applicationContext, LockOverlayActivity::class.java).apply {
                flags = Intent.FLAG_ACTIVITY_NEW_TASK or
                        Intent.FLAG_ACTIVITY_NO_ANIMATION or
                        Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                putExtra(LockOverlayActivity.EXTRA_PACKAGE_NAME, pkgName)
            }
            startActivity(intent)
        }
    }

    override fun onInterrupt() {
        Log.w(TAG, "Accessibility service interrupted")
    }
}
