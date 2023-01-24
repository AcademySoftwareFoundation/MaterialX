//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_RENDERHW_EXPORT_H
#define MATERIALX_RENDERHW_EXPORT_H

#include <MaterialXCore/Library.h>

/// @file
/// Macros for declaring imported and exported symbols.

#if defined(MATERIALX_RENDERHW_EXPORTS)
    #define MX_RENDERHW_API MATERIALX_SYMBOL_EXPORT
    #define MX_RENDERHW_EXTERN_TEMPLATE(...) MATERIALX_EXPORT_EXTERN_TEMPLATE(__VA_ARGS__)
#else
    #define MX_RENDERHW_API MATERIALX_SYMBOL_IMPORT
    #define MX_RENDERHW_EXTERN_TEMPLATE(...) MATERIALX_IMPORT_EXTERN_TEMPLATE(__VA_ARGS__)
#endif

#endif
