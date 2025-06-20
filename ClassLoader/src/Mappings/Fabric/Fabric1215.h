#pragma once
#include "../Mapping.h"

struct Fabric1215Mappings {
    static void setup() {
        Mapping::RegisterClass("Minecraft", "net.minecraft.class_310");

        Mapping::RegisterMethod("Minecraft", "getMinecraft", "method_1551", "()Lnet/minecraft/class_310;");
    }
};