#pragma once
#ifdef RegisterClass
#undef RegisterClass
#endif
#include "jni.h"
#include "../ClassLoader/ClassLoader.h"
#include <unordered_map>
#include <string>

struct MethodInfo {
    std::string name;
    std::string signature;
    jmethodID methodId;
};

class Mapping {
public:
    static void Initialize(JNIEnv* env_, ClassLoader* classLoader_);
    static void RegisterClass(const std::string& logicalName, const std::string& javaClassName);
    static void RegisterMethod(const std::string& classLogicalName,
        const std::string& methodLogicalName,
        const std::string& methodName,
        const std::string& signature);
    static jclass GetClass(const std::string& logicalName);
    static jmethodID GetMethod(const std::string& classLogicalName, const std::string& methodLogicalName);

private:
    static JNIEnv* env;
    static ClassLoader* classLoader;
    static std::unordered_map<std::string, std::string> classNames;
    static std::unordered_map<std::string, jclass> classes;
    static std::unordered_map<std::string, std::unordered_map<std::string, MethodInfo>> methods;
    static void CacheMethodIds();
};