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

static bool bool_test = false;
static int int_test = 69;
static float flt_test = 4.20f;
static char str_test[255]{0};

bool KeyboardCallback(void* data) {
    KeyboardEvent* e = (KeyboardEvent*)data;

    JINFO("Keyboard Event! (%d, %d)", e->key, e->state == BUTTONSTATE_DOWN);
    JINFO("%d, %d, %f, %s", bool_test, int_test, flt_test, str_test);

    return true;
}

// This function is called when the mod is loaded.
void __stdcall ModInit() {
    JAPI_ConfigRegisterBool(&bool_test, "bool_test", false);
    JAPI_ConfigRegisterInt(&int_test, "int_test", 69);
    JAPI_ConfigRegisterFloat(&flt_test, "flt_test", 4.20f);
    JAPI_ConfigRegisterString(str_test, "str_test", "cordMod");

    JAPI_RegisterEventCallback("KeyboardEvent", KeyboardCallback);

    JINFO("Initialized!");
}