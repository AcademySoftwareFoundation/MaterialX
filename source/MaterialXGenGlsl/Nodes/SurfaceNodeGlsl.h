//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SURFACENODEGLSL_H
#define MATERIALX_SURFACENODEGLSL_H

#include <MaterialXGenHw/Nodes/HwSurfaceNode.h>

MATERIALX_NAMESPACE_BEGIN

// This alias from SurfaceNodeGlsl to HwSurfaceNode is
// a backwards compatibility affordance for OpenUSD.
// The old SurfaceNodeGlsl class was removed in MatX 1.39.5
// once that becomes the minimum version for MaterialX
// in OpenUSD we can remove this.
using SurfaceNodeGlsl = HwSurfaceNode;

MATERIALX_NAMESPACE_END

#endif
