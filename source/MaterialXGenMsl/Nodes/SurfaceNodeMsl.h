//
// Created by Lee Kerley on 10/8/25.
//

#ifndef MATERIALX_SURFACENODEMSL_H
#define MATERIALX_SURFACENODEMSL_H

#include <MaterialXGenShader/Nodes/HwSurfaceNode.h>

MATERIALX_NAMESPACE_BEGIN

// This alias from SurfaceNodeGlsl to HwSurfaceNode is
// a backwards compatibility affordance for OpenUSD
// The old SurfaceNodeGlsl class was removed in MatX 1.39.5
// once that becomes the minimum version for MaterialX
// in OpenUSD we can remove this.
using SurfaceNodeMsl = HwSurfaceNode;

MATERIALX_NAMESPACE_END

#endif // MATERIALX_SURFACENODEMSL_H
