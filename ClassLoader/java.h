#pragma once
#include "jni.h"
#include "jvmti.h"

#include <mutex>
#include <unordered_map>
#include <string>
#include <iostream>
#include <memory>

class ClassLoader {
public:
    JNIEnv* env = nullptr;
    JavaVM* jvm = nullptr;
    jvmtiEnv* jvmti = nullptr;

    bool Initialize() {
        if (JNI_GetCreatedJavaVMs(&jvm, 1, nullptr) != JNI_OK || !jvm) {
            std::cerr << "[!] Failed to get JVM" << std::endl;
            return false;
        }

        if (jvm->GetEnv((void**)&env, JNI_VERSION_21) == JNI_EDETACHED) {
            if (jvm->AttachCurrentThread((void**)&env, nullptr) != JNI_OK) {
                std::cerr << "[!] Failed to attach thread" << std::endl;
                return false;
            }
        }

        if (jvm->GetEnv((void**)&jvmti, JVMTI_VERSION_21) != JNI_OK) {
            std::cerr << "[!] Failed to get JVMTI environment" << std::endl;
            return false;
        }

        return true;
    }

    void GetLoadedClasses() {
        if (!jvmti || !env) return;

        jclass* classes = nullptr;
        jint count = 0;

        jvmtiError err = jvmti->GetLoadedClasses(&count, &classes);
        if (err != JVMTI_ERROR_NONE) {
            std::cerr << "[!] GetLoadedClasses failed: " << err << std::endl;
            return;
        }

        jclass classClass = env->FindClass("java/lang/Class");
        if (!classClass) {
            jvmti->Deallocate((unsigned char*)classes);
            return;
        }

        jmethodID getName = env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
        if (!getName) {
            jvmti->Deallocate((unsigned char*)classes);
            return;
        }

        std::lock_guard<std::mutex> lock(classesMutex);
        loadedClasses.clear();

        for (int i = 0; i < count; i++) {
            jstring name = (jstring)env->CallObjectMethod(classes[i], getName);
            if (name) {
                const char* className = env->GetStringUTFChars(name, nullptr);
                if (className) {
                    std::string classNameStr(className);
                    jclass globalRef = (jclass)env->NewGlobalRef(classes[i]);
                    if (globalRef) {
                        loadedClasses[classNameStr] = globalRef;
                    }
                    env->ReleaseStringUTFChars(name, className);
                }
                env->DeleteLocalRef(name);
            }
        }

        jvmti->Deallocate((unsigned char*)classes);
    }

    jclass FindClass(const std::string& className) {
        std::lock_guard<std::mutex> lock(classesMutex);
        auto it = loadedClasses.find(className);
        return it != loadedClasses.end() ? it->second : nullptr;
    }

    ~ClassLoader() {
        std::lock_guard<std::mutex> lock(classesMutex);
        for (auto& pair : loadedClasses) {
            env->DeleteGlobalRef(pair.second);
        }
        loadedClasses.clear();

        if (jvm && env) {
            jvm->DetachCurrentThread();
        }
    }

private:
    std::unordered_map<std::string, jclass> loadedClasses;
    std::mutex classesMutex;
};

extern std::unique_ptr<ClassLoader> g_classLoader;