//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CORE_API_H
#define MATERIALX_CORE_API_H

/// @file
/// Macros for declaring imported and exported symbols.

#if defined(MATERIALX_BUILD_SHARED_LIBS)
    #if defined(_WIN32)
        #pragma warning(disable:4251)
        #pragma warning(disable:4275)
        #pragma warning(disable:4661)
        #if defined(MX_CORE_EXPORTS)
            #define MX_CORE_API __declspec(dllexport)
        #else
            #define MX_CORE_API __declspec(dllimport)
        #endif
    #else
        // Presently non-Windows platforms just export all symbols from
        // shared libraries rather than using the explicit declarations.
        #define MX_CORE_API
    #endif
#else
    #define MX_CORE_API
#endif

#endif
