package com.securelock.app

import android.util.Log

object NativeSecurityBridge {
    private const val TAG = "NativeSecurityBridge"

    init {
        try {
            System.loadLibrary("securelock_native")
            Log.i(TAG, "SecureLock native C++ library loaded successfully: ${getEngineVersion()}")
        } catch (e: UnsatisfiedLinkError) {
            Log.e(TAG, "Failed to load libsecurelock_native.so", e)
        }
    }

    /**
     * Retrieves the C++ engine version and architecture flags
     */
    external fun getEngineVersion(): String

    /**
     * Constant-time PIN hash verification executed in C++ stack memory
     */
    external fun verifyMasterPinNative(inputPin: String, storedHash: String): Boolean

    /**
     * Anti-tamper, debugger ptrace, and root detection run at native assembly level
     * Returns 0 if clean, negative error codes if environment is compromised.
     */
    external fun checkEnvironmentIntegrity(): Int

    /**
     * Native hardware-accelerated AES-256-GCM buffer encryption
     */
    external fun encryptVaultBufferNative(inputData: ByteArray, passphrase: String): ByteArray?

    /**
     * Native hardware-accelerated AES-256-GCM buffer decryption
     */
    external fun decryptVaultBufferNative(cipherData: ByteArray, passphrase: String): ByteArray?
}
