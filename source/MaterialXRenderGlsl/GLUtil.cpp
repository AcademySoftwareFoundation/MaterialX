//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/External/GLew/glew.h>

#include <MaterialXRenderGlsl/GLUtil.h>

#include <iostream>

namespace MaterialX
{

void checkGlErrors(const string& context)
{
    for (GLenum error = glGetError(); error; error = glGetError())
    {
        std::cerr << "OpenGL error " << context << ": " << std::to_string(error) << std::endl;
    }
}

} // namespace MaterialX
