//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/GenOptions.h>

namespace MaterialX
{

GenOptions::GenOptions() :
    shaderInterfaceType(SHADER_INTERFACE_COMPLETE),
    fileTextureVerticalFlip(false),
    hwTransparency(false),
    hwSpecularEnvironmentMethod(SPECULAR_ENVIRONMENT_FIS),
    hwMaxActiveLightSources(3),
    hwNormalizeUdimTexCoords(false),
    textureSpaceRender(false)
{
}
GenOptions::~GenOptions()
{
}

} // namespace MaterialX
