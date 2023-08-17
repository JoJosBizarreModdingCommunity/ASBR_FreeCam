#include "main.h"

#include <stdio.h>

// This function is called when the mod is loaded.
// It should return a ModMeta struct with the mod's information.
ModMeta __stdcall GetModInfo() {
    static ModMeta meta = {
        "Test Mod", // Name
        "TestMod", // GUID
        "1.0.0", // Version
        "Kapilarny" // Author
    };

    return meta;
}

// This function is called when the mod is loaded.
void __stdcall ModInit() {
    JAPI_LogInfo("Initialized!");
}