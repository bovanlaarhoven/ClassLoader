#include "ClassLoader.h"
#include <iostream>

bool ClassLoader::Initialize() {
    if (JNI_GetCreatedJavaVMs(&jvm, 1, nullptr) != JNI_OK || !jvm) {
        std::cerr << "[!] Failed to get JVM" << std::endl;
        return false;
    }

    if (jvm->GetEnv((void**)&env, JNI_VERSION_1_8) == JNI_EDETACHED) {
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

void ClassLoader::GetLoadedClasses() {
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

jclass ClassLoader::FindClass(const std::string& className) {
    std::lock_guard<std::mutex> lock(classesMutex);
    auto it = loadedClasses.find(className);
    if (it != loadedClasses.end()) {
        return it->second;
    }

    jstring jClassName = env->NewStringUTF(className.c_str());

    jclass classLoaderClass = env->FindClass("java/lang/ClassLoader");
    jmethodID getSystemClassLoader = env->GetStaticMethodID(classLoaderClass, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
    jobject systemClassLoader = env->CallStaticObjectMethod(classLoaderClass, getSystemClassLoader);

    jmethodID loadClass = env->GetMethodID(classLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    jclass clazz = (jclass)env->CallObjectMethod(systemClassLoader, loadClass, jClassName);
    env->DeleteLocalRef(jClassName);

    if (clazz) {
        jclass globalRef = (jclass)env->NewGlobalRef(clazz);
        loadedClasses[className] = globalRef;
        return globalRef;
    }

    return nullptr;
}

ClassLoader::~ClassLoader() {
    std::lock_guard<std::mutex> lock(classesMutex);
    for (auto& pair : loadedClasses) {
        env->DeleteGlobalRef(pair.second);
    }
    loadedClasses.clear();

    if (jvm && env) {
        jvm->DetachCurrentThread();
    }
}
