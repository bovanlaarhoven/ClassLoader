#pragma once

#include "jni.h"
#include "jvmti.h"

#include <mutex>
#include <unordered_map>
#include <string>
#include <memory>

class ClassLoader {
public:
    JNIEnv* env = nullptr;
    JavaVM* jvm = nullptr;
    jvmtiEnv* jvmti = nullptr;

    bool Initialize();
    void GetLoadedClasses();
    jclass FindClass(const std::string& className);

    ~ClassLoader();

private:
    std::unordered_map<std::string, std::string> classNames;
    std::unordered_map<std::string, jclass> classes;
    std::unordered_map<std::string, jclass> loadedClasses;
    std::mutex classesMutex;
};

extern std::unique_ptr<ClassLoader> g_classLoader;
