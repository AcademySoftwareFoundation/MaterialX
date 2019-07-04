//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOgsFx/MayaGlslPluginShaderGenerator.h>
#include <MaterialXGenOgsFx/OgsFxSyntax.h>

#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

const string MayaGlslPluginShaderGenerator::TARGET = "ogsfx_mayaglslplugin";

MayaGlslPluginShaderGenerator::MayaGlslPluginShaderGenerator() :
    OgsFxShaderGenerator()
{
    // Update view position semantic to match Maya's semantics.
    _semanticsMap[HW::T_VIEW_POSITION] = "ViewPosition";
}

ShaderGeneratorPtr MayaGlslPluginShaderGenerator::create()
{
    return std::make_shared<MayaGlslPluginShaderGenerator>();
}

void MayaGlslPluginShaderGenerator::getTechniqueParams(const Shader& shader, string& params) const
{
    if (shader.hasAttribute(HW::ATTR_TRANSPARENT))
    {
        params = "string transparency = \"transparent\";";
    }
}

} // namespace MaterialX
