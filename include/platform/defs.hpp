#if defined(EMSCRIPTEN)
	#define PLATFORM_WEB
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__MINGW32__) || defined(__MINGW64__)
    #define PLATFORM_WIN
    #define PLATFORM_NATIVE
#elif defined(__APPLE__) && defined(__MACH__)
	#define PLATFORM_MAC
	#define PLATFORM_NATIVE
#elif defined(__linux__) || defined(__unix) || defined(__unix__)
	#define PLATFORM_UNIX
	#define PLATFORM_NATIVE
#else
	#error "The current platform is not recognized. (platform/defs.hpp)"
#endif