//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_HARDWAREPLATFORM_H
//
// Platform macros. All platform specific files in the module should use the OS*_ macros
// instead of the native ones.
//
#if defined(_WIN64) || defined(_WIN32)
#define OSWin_
#elif defined(__linux__)
#define OSLinux_
#elif defined(__APPLE__)
#define OSMac_ 
#else
#define OSUnsupported_
#endif

#endif
