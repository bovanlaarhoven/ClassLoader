#include "MinecraftDetector.h"
#include <iostream>

ModLoader MinecraftDetector::DetectModLoader(ClassLoader* classLoader) {
    if (HasClass(classLoader, "net.fabricmc.api.ModInitializer") ||
        HasClass(classLoader, "net.fabricmc.loader.api.FabricLoader") ||
        HasClass(classLoader, "net.fabricmc.loader.launch.knot.Knot")) {
        std::cout << "[*] Detected Fabric mod loader" << std::endl;
        return ModLoader::FABRIC;
    }

    if (HasClass(classLoader, "org.quiltmc.loader.api.QuiltLoader") ||
        HasClass(classLoader, "org.quiltmc.qsl.base.api.entrypoint.ModInitializer") ||
        HasClass(classLoader, "org.quiltmc.loader.impl.QuiltLoaderImpl")) {
        std::cout << "[*] Detected Quilt mod loader" << std::endl;
        return ModLoader::QUILT;
    }

    if (HasClass(classLoader, "net.minecraftforge.fml.common.Mod") ||
        HasClass(classLoader, "net.minecraftforge.common.MinecraftForge") ||
        HasClass(classLoader, "net.minecraftforge.fml.ModLoader") ||
        HasClass(classLoader, "net.minecraftforge.fml.loading.FMLLoader")) {
        std::cout << "[*] Detected Forge mod loader" << std::endl;
        return ModLoader::FORGE;
    }

    if (HasClass(classLoader, "net.neoforged.fml.common.Mod") ||
        HasClass(classLoader, "net.neoforged.neoforge.common.NeoForge") ||
        HasClass(classLoader, "net.neoforged.neoforge.NeoForgeLoader")) {
        std::cout << "[*] Detected NeoForge mod loader" << std::endl;
        return ModLoader::NEOFORGE;
    }

    std::cout << "[*] No mod loader detected (Vanilla)" << std::endl;
    return ModLoader::VANILLA;
}

bool MinecraftDetector::HasClass(ClassLoader* classLoader, const std::string& className) {
    jclass clazz = classLoader->FindClass(className);
    if (clazz == nullptr) {
        std::cout << "[DEBUG] Class not found: " << className << std::endl;
        return false;
    }
    std::cout << "[DEBUG] Class found: " << className << std::endl;
    return true;
}
