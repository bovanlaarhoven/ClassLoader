#pragma once
#include "../Mapping.h"

struct Vanilla1215Mappings {
    static void setup() {
        Mapping::RegisterClass("Minecraft", "fqq");

        Mapping::RegisterMethod("Minecraft", "getMinecraft", "Q", "()Lfqq;");
    }
};