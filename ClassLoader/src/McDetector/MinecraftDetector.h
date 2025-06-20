#pragma once
#include <string>
#include "../ClassLoader/ClassLoader.h"

enum class ModLoader {
    VANILLA,
    FABRIC,
    FORGE,
    QUILT,
    NEOFORGE,
    UNKNOWN
};

class MinecraftDetector {
public:
    static ModLoader DetectModLoader(ClassLoader* classLoader);

private:
    static bool HasClass(ClassLoader* classLoader, const std::string& className);
};
