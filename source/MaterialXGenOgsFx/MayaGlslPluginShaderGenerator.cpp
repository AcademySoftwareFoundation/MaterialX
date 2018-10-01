#include <MaterialXGenOgsFx/MayaGlslPluginShaderGenerator.h>
#include <MaterialXGenOgsFx/OgsFxSyntax.h>

namespace MaterialX
{

namespace
{
    static const std::string VIEW_POSITON_UNIFORM_NAME = "u_viewPosition";
    static const std::string VIEW_POSITON_SEMATIC = "ViewPosition";
}

MayaGlslPluginShader::MayaGlslPluginShader(const string& name)
    : ParentClass(name)
{
}

void MayaGlslPluginShader::createUniform(size_t stage, const string& block, const TypeDesc* type, const string& name, const string& semantic, ValuePtr value)
{
    // If no semantic is given and this is the view position uniform
    // we need to override its default semantic
    if (semantic.empty() && name == VIEW_POSITON_UNIFORM_NAME)
    {
        HwShader::createUniform(stage, block, type, name, VIEW_POSITON_SEMATIC, value);
        return;
    }
    ParentClass::createUniform(stage, block, type, name, semantic, value);
}

void MayaGlslPluginShader::createAppData(const TypeDesc* type, const string& name, const string& semantic)
{
    // If no semantic is given and this is the view position uniform
    // we need to override its default semantic
    if (semantic.empty() && name == VIEW_POSITON_UNIFORM_NAME)
    {
        HwShader::createAppData(type, name, VIEW_POSITON_SEMATIC);
        return;
    }
    ParentClass::createAppData(type, name, semantic);
}

void MayaGlslPluginShader::createVertexData(const TypeDesc* type, const string& name, const string& semantic)
{
    // If no semantic is given and this is the view position uniform
    // we need to override its default semantic
    if (semantic.empty() && name == VIEW_POSITON_UNIFORM_NAME)
    {
        HwShader::createVertexData(type, name, VIEW_POSITON_SEMATIC);
        return;
    }
    ParentClass::createVertexData(type, name, semantic);
}


const string MayaGlslPluginShaderGenerator::TARGET = "ogsfx_mayaglslplugin";

OgsFxShaderPtr MayaGlslPluginShaderGenerator::createShader(const string& name)
{
    return std::make_shared<MayaGlslPluginShader>(name);
}

void MayaGlslPluginShaderGenerator::getTechniqueParams(const Shader& shader, string& params)
{
    const HwShader& hwShader = static_cast<const HwShader&>(shader);
    if (hwShader.hasTransparency())
    {
        params = "string transparency = \"transparent\";";
    }
}

} // namespace MaterialX
