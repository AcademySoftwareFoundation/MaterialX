#ifndef MATERIALX_HARDWAREPLATFORM_H
//
// Platform macros. All platform specific files in the module should use the OS*_ macros
// instead of the native ones.
//
#if defined(_WIN64)
#define OSWin_
#elif defined(__linux__)
#define OSLinux_
#elif defined(__APPLE__)
#define OSMac_ 
#else
#define OSUnsupported_
#endif

#endif
