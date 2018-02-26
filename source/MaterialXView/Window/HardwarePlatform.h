#ifndef MATERIALX_HARDWAREPLATFORM_H

// Platform macros. Only Windows platform is supported at this point
// Linux and Mac flags well be enabled when fully supported.
#if defined(_WIN64)
//#define OSUnsupported_
#define OSWin_
#elif defined(__linux__)
#define OSUnsupported_
//#define OSLinux_
#elif defined(__APPLE__)
#define OSUnsupported_
//#define OSMac_ 
#else
#define OSUnsupported_
#endif

#endif
