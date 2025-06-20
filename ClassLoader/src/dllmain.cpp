#include <Windows.h>
#include <iostream>
#include "../src/ClassLoader/ClassLoader.h"
#include "../src/Mappings/Mapping.h"
#include "../src/Mappings/Fabric/Fabric1215.h"
#include "../src/McDetector/MinecraftDetector.h"

FILE* g_consoleOutput = nullptr;
std::unique_ptr<ClassLoader> g_classLoader;
std::unique_ptr<Mapping> g_mapping;

void MainThread(HMODULE module) {
    AllocConsole();
    freopen_s(&g_consoleOutput, "CONOUT$", "w", stdout);
    std::cout << "[*] DLL Injected" << std::endl;

    g_classLoader = std::make_unique<ClassLoader>();
    if (!g_classLoader->Initialize()) {
        std::cerr << "[!] Failed to initialize ClassLoader" << std::endl;
        FreeLibraryAndExitThread(module, 0);
        return;
    }
    std::cout << "[*] JVM, JNI, and JVMTI environments acquired!" << std::endl;

    std::cout << "[*] Detecting mod loader..." << std::endl;
    ModLoader modLoader = MinecraftDetector::DetectModLoader(g_classLoader.get());

    std::cout << "\n=== MOD LOADER DETECTION RESULT ===" << std::endl;
    std::cout << "Mod Loader: ";
    switch (modLoader) {
    case ModLoader::VANILLA: std::cout << "Vanilla"; break;
    case ModLoader::FABRIC: std::cout << "Fabric"; break;
    case ModLoader::FORGE: std::cout << "Forge"; break;
    case ModLoader::QUILT: std::cout << "Quilt"; break;
    case ModLoader::NEOFORGE: std::cout << "NeoForge"; break;
    default: std::cout << "Unknown"; break;
    }
    std::cout << std::endl;
    std::cout << "==================================\n" << std::endl;

    g_classLoader->GetLoadedClasses();
    std::cout << "[*] Loaded classes cached" << std::endl;

    g_mapping = std::make_unique<Mapping>();

    Fabric1215Mappings::setup();

    g_mapping->Initialize(g_classLoader->env, g_classLoader.get());

    jclass minecraftClass = g_mapping->GetClass("Minecraft");
    if (!minecraftClass) {
        std::cerr << "[!] Failed to find Minecraft class via Mapping" << std::endl;
        FreeLibraryAndExitThread(module, 0);
        return;
    }
    std::cout << "[*] Found Minecraft class" << std::endl;

    jmethodID getMinecraftMethod = g_mapping->GetMethod("Minecraft", "getMinecraft");
    if (!getMinecraftMethod) {
        std::cerr << "[!] Failed to find getMinecraft method via Mapping" << std::endl;
        FreeLibraryAndExitThread(module, 0);
        return;
    }
    std::cout << "[*] Found getMinecraft method" << std::endl;

    jobject minecraftInstance = g_classLoader->env->CallStaticObjectMethod(minecraftClass, getMinecraftMethod);
    if (!minecraftInstance) {
        std::cerr << "[!] Failed to call getMinecraft or returned null" << std::endl;
    }
    else {
        std::cout << "[*] Successfully called getMinecraft" << std::endl;
        jobject globalMinecraftInstance = g_classLoader->env->NewGlobalRef(minecraftInstance);
        g_classLoader->env->DeleteGlobalRef(globalMinecraftInstance);
    }

    while (!(GetAsyncKeyState(VK_DELETE) & 1)) {
        Sleep(100);
    }

    std::cout << "[*] Uninjecting" << std::endl;
    if (g_consoleOutput) fclose(g_consoleOutput);
    FreeConsole();
    FreeLibraryAndExitThread(module, 0);
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, module, 0, nullptr);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        g_mapping.reset();
        g_classLoader.reset();
    }
    return TRUE;
}
