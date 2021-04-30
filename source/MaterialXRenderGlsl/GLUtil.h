//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GLUTIL_H
#define MATERIALX_GLUTIL_H

/// @file
/// OpenGL utilities

#include <MaterialXRenderGlsl/Export.h>

#include <MaterialXCore/Library.h>

namespace MaterialX
{

MX_RENDERGLSL_API void checkGlErrors(const string& context);

} // namespace MaterialX

#endif
