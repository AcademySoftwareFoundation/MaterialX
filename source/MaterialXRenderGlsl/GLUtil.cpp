//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/External/Glad/glad.h>

#include <MaterialXRenderGlsl/GLUtil.h>

#include <iostream>

MATERIALX_NAMESPACE_BEGIN

void checkGlErrors(const string& context)
{
    for (GLenum error = glGetError(); error; error = glGetError())
    {
        std::cerr << "OpenGL error " << context << ": " << std::to_string(error) << std::endl;
    }
}

MATERIALX_NAMESPACE_END
