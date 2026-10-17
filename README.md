# SecureLock - Android APK Security Suite
High-performance Android App Locker with C++ NDK Engine integration.

## Features
- **C++ NDK Engine**: libsecurelock_native.so with constant-time PIN hash verification, anti-tampering ptrace checks, and AES-256-GCM crypto.
- **Real-Time App Locker**: Accessibility & Foreground Service package interceptor.
- **Intruder Detection**: Front-camera snapshot logging on repeated failed unlock attempts.
- **Disguise Mode**: Fake system crash dialog bypassable with secret gesture.
- **Biometric & Pattern**: BiometricPrompt API & vector pattern lock support.

## Building the APK
1. Open this directory in **Android Studio Hedgehog / Ladybug (2024+)**.
2. Ensure Android NDK `26.1.10909125` and CMake `3.22.1+` are installed via SDK Manager.
3. Run `./gradlew assembleRelease` or click **Build > Build Bundle(s) / APK(s) > Build APK(s)**.
