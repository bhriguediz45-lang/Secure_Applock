/**
 * SecureLock Native Security Core (NDK / C++17)
 * Package: com.securelock.app
 * Hardware-accelerated cryptographic primitives, anti-debug/tamper detection,
 * and memory zeroing to mitigate memory scraping attacks.
 */

#include <jni.h>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <android/log.h>
#include "crypto_vault.hpp"

#define TAG "SecureLock_NDK"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static const char* NATIVE_SALT = "SecureLock_KDF_Salt_2026";

/**
 * Constant-time byte array comparison (timing-attack defense)
 * Never short-circuits on mismatch.
 */
static bool constant_time_memcmp(const uint8_t* a, const uint8_t* b, size_t length) {
    volatile uint8_t result = 0;
    for (size_t i = 0; i < length; ++i) {
        result |= (a[i] ^ b[i]);
    }
    return result == 0;
}

/**
 * Explicit volatile zeroing of sensitive buffers in memory
 */
static void secure_zero_memory(void* ptr, size_t len) {
    volatile uint8_t* p = static_cast<volatile uint8_t*>(ptr);
    while (len--) {
        *p++ = 0;
    }
}

/**
 * Anti-debugging & Environment Integrity Check:
 * - ptrace attachment check
 * - /proc/self/status TracerPid analysis
 * - Root detection via known su binaries
 */
static int check_integrity_internal() {
    // 1. Anti-ptrace: if debugger is attached, ptrace(PTRACE_TRACEME) fails
    if (ptrace(PTRACE_TRACEME, 0, 1, 0) < 0) {
        LOGW("Tamper detected: ptrace attachment detected!");
        return -1; // Debugger attached
    }

    // 2. Read /proc/self/status TracerPid
    int fd = open("/proc/self/status", O_RDONLY);
    if (fd != -1) {
        char buffer[1024];
        ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            const char* tracer = strstr(buffer, "TracerPid:");
            if (tracer) {
                int tracerPid = 0;
                if (sscanf(tracer, "TracerPid:\t%d", &tracerPid) == 1 && tracerPid != 0) {
                    LOGW("Tamper detected: TracerPid = %d", tracerPid);
                    return -2; // Debugged
                }
            }
        }
    }

    // 3. Su binary checks
    const char* suPaths[] = {
        "/system/bin/su",
        "/system/xbin/su",
        "/sbin/su",
        "/system/sd/xbin/su",
        "/system/bin/failsafe/su",
        "/data/local/xbin/su",
        "/data/local/bin/su"
    };

    for (const char* path : suPaths) {
        struct stat st;
        if (stat(path, &st) == 0) {
            LOGW("Root presence detected at %s", path);
            return -3; // Root detected
        }
    }

    return 0; // Device environment clean
}

extern "C" {

JNIEXPORT jstring JNICALL
Java_com_securelock_app_NativeSecurityBridge_getEngineVersion(
        JNIEnv* env,
        jobject /* this */) {
    std::string version = "SecureLock NDK Core v2.4 (C++17/ARM64-v8a hardened)";
    return env->NewStringUTF(version.c_str());
}

JNIEXPORT jboolean JNICALL
Java_com_securelock_app_NativeSecurityBridge_verifyMasterPinNative(
        JNIEnv* env,
        jobject /* this */,
        jstring inputPin_,
        jstring storedHash_) {
    if (!inputPin_ || !storedHash_) return JNI_FALSE;

    const char* inputPin = env->GetStringUTFChars(inputPin_, nullptr);
    const char* storedHash = env->GetStringUTFChars(storedHash_, nullptr);

    // Compute hardware-salted cryptographic hash
    std::string computed = securelock::derive_hash(inputPin, NATIVE_SALT);

    size_t hashLen = strlen(storedHash);
    bool match = false;
    if (computed.length() == hashLen) {
        match = constant_time_memcmp(
            reinterpret_cast<const uint8_t*>(computed.c_str()),
            reinterpret_cast<const uint8_t*>(storedHash),
            hashLen
        );
    }

    // Zero out memory to defeat RAM scrapers
    secure_zero_memory(const_cast<char*>(inputPin), strlen(inputPin));

    env->ReleaseStringUTFChars(inputPin_, inputPin);
    env->ReleaseStringUTFChars(storedHash_, storedHash);

    return match ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jint JNICALL
Java_com_securelock_app_NativeSecurityBridge_checkEnvironmentIntegrity(
        JNIEnv* /* env */,
        jobject /* this */) {
    return check_integrity_internal();
}

JNIEXPORT jbyteArray JNICALL
Java_com_securelock_app_NativeSecurityBridge_encryptVaultBufferNative(
        JNIEnv* env,
        jobject /* this */,
        jbyteArray inputData_,
        jstring passphrase_) {
    if (!inputData_ || !passphrase_) return nullptr;

    jsize len = env->GetArrayLength(inputData_);
    jbyte* buffer = env->GetByteArrayElements(inputData_, nullptr);
    const char* pass = env->GetStringUTFChars(passphrase_, nullptr);

    std::vector<uint8_t> plain(buffer, buffer + len);
    std::vector<uint8_t> encrypted = securelock::aes_256_gcm_encrypt(plain, pass, NATIVE_SALT);

    secure_zero_memory(buffer, len);
    env->ReleaseByteArrayElements(inputData_, buffer, 0);
    env->ReleaseStringUTFChars(passphrase_, pass);

    jbyteArray output = env->NewByteArray(static_cast<jsize>(encrypted.size()));
    env->SetByteArrayRegion(output, 0, static_cast<jsize>(encrypted.size()), reinterpret_cast<const jbyte*>(encrypted.data()));
    return output;
}

JNIEXPORT jbyteArray JNICALL
Java_com_securelock_app_NativeSecurityBridge_decryptVaultBufferNative(
        JNIEnv* env,
        jobject /* this */,
        jbyteArray cipherData_,
        jstring passphrase_) {
    if (!cipherData_ || !passphrase_) return nullptr;

    jsize len = env->GetArrayLength(cipherData_);
    jbyte* buffer = env->GetByteArrayElements(cipherData_, nullptr);
    const char* pass = env->GetStringUTFChars(passphrase_, nullptr);

    std::vector<uint8_t> cipher(buffer, buffer + len);
    std::vector<uint8_t> decrypted = securelock::aes_256_gcm_decrypt(cipher, pass, NATIVE_SALT);

    env->ReleaseByteArrayElements(cipherData_, buffer, 0);
    env->ReleaseStringUTFChars(passphrase_, pass);

    if (decrypted.empty()) {
        return nullptr; // Decryption failed or authentication tag mismatch
    }

    jbyteArray output = env->NewByteArray(static_cast<jsize>(decrypted.size()));
    env->SetByteArrayRegion(output, 0, static_cast<jsize>(decrypted.size()), reinterpret_cast<const jbyte*>(decrypted.data()));
    return output;
}

} // extern "C"
