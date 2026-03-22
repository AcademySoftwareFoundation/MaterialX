//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GENSHADER2_DATAHANDLE_H
#define MATERIALX_GENSHADER2_DATAHANDLE_H

/// @file
/// Opaque 64-bit handle type used as the currency of the IShaderSource interface.
///
/// A DataHandle is an opaque token whose meaning is defined entirely by the
/// IShaderSource implementation that produced it.  Implementations may store
/// a raw pointer (cast to uint64_t), a string hash, an integer index, or any
/// other compact representation that fits in 64 bits.
///
/// Handles are valid only for the lifetime of the IShaderSource that created
/// them.  They must not be shared across IShaderSource instances.

#include <MaterialXGenShader2/Export.h>

#include <cstdint>

MATERIALX_NAMESPACE_BEGIN

/// Opaque 64-bit identifier representing any object in a shader source graph
/// (node, input port, output port, NodeDef, etc.).
using DataHandle = uint64_t;

/// Sentinel value indicating an absent or invalid handle.
static constexpr DataHandle InvalidHandle = 0;

/// Returns true if the handle refers to a valid object.
inline bool isValidHandle(DataHandle h) noexcept { return h != InvalidHandle; }

MATERIALX_NAMESPACE_END

#endif // MATERIALX_GENSHADER2_DATAHANDLE_H
