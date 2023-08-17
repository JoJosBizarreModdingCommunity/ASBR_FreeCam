#include "main.h"

#include <stdio.h>

ModMeta __stdcall GetModInfo() {
    static ModMeta meta = {
        "Test Mod", // Name
        "TestMod", // GUID
        "1.0.0", // Version
        "Kapilarny" // Author
    };

    return meta;
}

void __stdcall ModInit() {
    JAPI_LogInfo("Initialized!");
}