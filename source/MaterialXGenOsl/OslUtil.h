//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_OSLUTIL_H
#define MATERIALX_OSLUTIL_H

/// @file
/// OSL shading language generator

#include <MaterialXGenOsl/Export.h>

MATERIALX_NAMESPACE_BEGIN

class ShaderGraph;
class Document;
using ConstDocumentPtr = shared_ptr<const Document>;
class TypeSystem;
using TypeSystemPtr = shared_ptr<TypeSystem>;
class GenContext;

/// Write docs
MX_GENOSL_API void addSetCiTerminalNode(ShaderGraph& graph, ConstDocumentPtr document, TypeSystemPtr typeSystem, GenContext& context);

MATERIALX_NAMESPACE_END

#endif
