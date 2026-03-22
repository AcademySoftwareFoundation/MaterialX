//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GENSHADER2_EXPORT_H
#define MATERIALX_GENSHADER2_EXPORT_H

#include <MaterialXGenShader2/Library.h>

/// @file
/// Macros for declaring imported and exported symbols.

#if defined(MATERIALX_GENSHADER2_EXPORTS)
    #define MX_GENSHADER2_API MATERIALX_SYMBOL_EXPORT
    #define MX_GENSHADER2_EXTERN_TEMPLATE(...) MATERIALX_EXPORT_EXTERN_TEMPLATE(__VA_ARGS__)
#else
    #define MX_GENSHADER2_API MATERIALX_SYMBOL_IMPORT
    #define MX_GENSHADER2_EXTERN_TEMPLATE(...) MATERIALX_IMPORT_EXTERN_TEMPLATE(__VA_ARGS__)
#endif

#endif // MATERIALX_GENSHADER2_EXPORT_H
