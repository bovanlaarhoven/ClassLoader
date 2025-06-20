#include <Windows.h>
#include <iostream>
#include "java.h"

FILE* g_consoleOutput = nullptr;
std::unique_ptr<ClassLoader> g_classLoader;

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

    g_classLoader->GetLoadedClasses();
    std::cout << "[*] Loaded classes cached" << std::endl;

    const std::string minecraftClassName = "net.minecraft.class_310";

    jclass minecraftClass = g_classLoader->FindClass(minecraftClassName);
    if (!minecraftClass) {
        std::cerr << "[!] Failed to find Minecraft class: " << minecraftClassName << std::endl;
        FreeLibraryAndExitThread(module, 0);
        return;
    }
    std::cout << "[*] Found Minecraft class: " << minecraftClassName << std::endl;

    jmethodID getMinecraftMethod = g_classLoader->env->GetStaticMethodID(
        minecraftClass,
        "method_1551",
        "()Lnet/minecraft/class_310;"
    );

    if (!getMinecraftMethod) {
        std::cerr << "[!] Failed to find getMinecraft method" << std::endl;
        FreeLibraryAndExitThread(module, 0);
        return;
    }
    std::cout << "[*] Found getMinecraft method" << std::endl;

    jobject minecraftInstance = g_classLoader->env->CallStaticObjectMethod(minecraftClass, getMinecraftMethod);
    if (!minecraftInstance) {
        std::cerr << "[!] Failed to call getMinecraft method or returned null" << std::endl;
    }
    else {
        std::cout << "[*] Successfully called getMinecraft" << std::endl;

        jobject globalMinecraftInstance = g_classLoader->env->NewGlobalRef(minecraftInstance);
        g_classLoader->env->DeleteGlobalRef(globalMinecraftInstance);
    }

    // Wait for unload trigger
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
        g_classLoader.reset();
    }
    return TRUE;
}