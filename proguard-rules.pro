## SecureLock ProGuard & R8 Obfuscation Rules

# Preserve JNI methods and native signatures
-keepclasseswithmembernames,includedescriptorclasses class * {
    native <methods>;
}

-keep class com.securelock.app.NativeSecurityBridge {
    *;
}

# Preserve Biometric and Accessibility Services
-keep class com.securelock.app.service.** { *; }
-keep class com.securelock.app.ui.LockOverlayActivity { *; }

# Obfuscate internal business rules and key derivation algorithms
-repackageclasses 'com.securelock.app.obfuscated'
-allowaccessmodification
-repackageclasses
-flattenpackagehierarchy

# Remove debug log calls in release APK
-assumenosideeffects class android.util.Log {
    public static boolean isLoggable(java.lang.String, int);
    public static int v(...);
    public static int d(...);
}
