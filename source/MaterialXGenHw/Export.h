//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GENHW_EXPORT_H
#define MATERIALX_GENHW_EXPORT_H

#include <MaterialXGenShader/Library.h>

/// @file
/// Macros for declaring imported and exported symbols.

#if defined(MATERIALX_GENHW_EXPORTS)
    #define MX_GENHW_API MATERIALX_SYMBOL_EXPORT
    #define MX_GENHW_EXTERN_TEMPLATE(...) MATERIALX_EXPORT_EXTERN_TEMPLATE(__VA_ARGS__)
#else
    #define MX_GENHW_API MATERIALX_SYMBOL_IMPORT
    #define MX_GENHW_EXTERN_TEMPLATE(...) MATERIALX_IMPORT_EXTERN_TEMPLATE(__VA_ARGS__)
#endif

#endif
