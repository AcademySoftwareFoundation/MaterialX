//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RENDEROSL_EXPORT_H
#define MATERIALX_RENDEROSL_EXPORT_H

#include <MaterialXCore/Library.h>

/// @file
/// Macros for declaring imported and exported symbols.

#if defined(MATERIALX_RENDEROSL_EXPORTS)
    #define MX_RENDEROSL_API MATERIALX_SYMBOL_EXPORT
    #define MX_RENDEROSL_EXTERN_TEMPLATE(...) MATERIALX_EXPORT_EXTERN_TEMPLATE(__VA_ARGS__)
#else
    #define MX_RENDEROSL_API MATERIALX_SYMBOL_IMPORT
    #define MX_RENDEROSL_EXTERN_TEMPLATE(...) MATERIALX_IMPORT_EXTERN_TEMPLATE(__VA_ARGS__)
#endif

#endif
