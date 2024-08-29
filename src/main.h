#pragma once

#define EXPORT extern "C" __declspec(dllexport)

#include <JojoAPI.h>

#define DEBUG_BUILD true // either true or false

EXPORT ModMeta __stdcall GetModInfo();
EXPORT void __stdcall ModInit();

// Logger macros

#define JFATAL(message, ...) JAPI_LogFatal(message, ##__VA_ARGS__);
#define JERROR(message, ...) JAPI_LogError(message, ##__VA_ARGS__);
#define JWARN(message, ...) JAPI_LogWarn(message, ##__VA_ARGS__);
#define JINFO(message, ...) JAPI_LogInfo(message, ##__VA_ARGS__);

#if DEBUG_BUILD == true
#define JDEBUG(message, ...) JAPI_LogDebug(message, ##__VA_ARGS__);
#define JTRACE(message, ...) JAPI_LogTrace(message, ##__VA_ARGS__);
#else
#define JDEBUG(message, ...)
#define JTRACE(message, ...)
#endif