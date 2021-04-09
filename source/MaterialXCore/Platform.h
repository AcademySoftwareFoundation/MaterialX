//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CORE_PLATFORM_H
#define MATERIALX_CORE_PLATFORM_H

/// @file
/// Macros for declaring imported and exported symbols.

#if defined(MATERIALX_BUILD_SHARED_LIBS)
    #if defined(_WIN32)
        #pragma warning(disable:4251)
        #pragma warning(disable:4275)
        #pragma warning(disable:4661)
        #define MATERIALX_SYMBOL_EXPORT __declspec(dllexport)
        #define MATERIALX_SYMBOL_IMPORT __declspec(dllimport)
        #define MATERIALX_EXPORT_EXTERN_TEMPLATE(...) template class MATERIALX_SYMBOL_EXPORT __VA_ARGS__
        #define MATERIALX_IMPORT_EXTERN_TEMPLATE(...) extern template class MATERIALX_SYMBOL_IMPORT __VA_ARGS__
    #else
        // Presently non-Windows platforms just export all symbols from
        // shared libraries rather than using the explicit declarations.
        #define MATERIALX_SYMBOL_EXPORT
        #define MATERIALX_SYMBOL_IMPORT
        #define MATERIALX_EXPORT_EXTERN_TEMPLATE(...)
        #define MATERIALX_IMPORT_EXTERN_TEMPLATE(...)
    #endif
#else
    #define MATERIALX_SYMBOL_EXPORT
    #define MATERIALX_SYMBOL_IMPORT
    #define MATERIALX_EXPORT_EXTERN_TEMPLATE(...)
    #define MATERIALX_IMPORT_EXTERN_TEMPLATE(...)
#endif

#endif
