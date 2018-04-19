#include <MaterialXShaderGen/ShaderGenerators/Glsl/SurfaceShaderGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>

namespace MaterialX
{

SgImplementationPtr SurfaceShaderGlsl::create()
{
    return std::make_shared<SurfaceShaderGlsl>();
}

const string& SurfaceShaderGlsl::getLanguage() const
{
    return GlslShaderGenerator::LANGUAGE;
}

const string& SurfaceShaderGlsl::getTarget() const
{
    return GlslShaderGenerator::TARGET;
}

void SurfaceShaderGlsl::createVariables(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    // TODO: 
    // The surface shader needs position, view and light sources. We should solve this by adding some 
    // dependency mechanism so this implementation can be set to depend on the PositionGlsl,  
    // ViewDirectionGlsl and LightGlsl nodes instead? This is where the MaterialX attribute "internalgeomprops" 
    // is needed.
    //
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createAppData(DataType::VECTOR3, "i_position");
    shader.createVertexData(DataType::VECTOR3, "positionWorld");
    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, DataType::VECTOR3, "u_viewDirection");
    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, DataType::INTEGER, "u_numActiveLightSources",
        EMPTY_STRING, Value::createValue<int>(0));
}

void SurfaceShaderGlsl::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (!shader.isCalculated("positionWorld"))
        {
            shader.setCalculated("positionWorld");
            shader.addLine("vd.positionWorld = hPositionWorld.xyz");
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        SourceCode::emitFunctionCall(node, shadergen, shader_);
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

bool SurfaceShaderGlsl::isTransparent(const SgNode& node) const
{
    // TODO: Support transparency, refraction, etc.
    if (node.getInput("opacity"))
    {
        MaterialX::ValuePtr value = node.getInput("opacity")->value;
        if (value)
        {
            try
            {
                MaterialX::Color3 color3Value = value->asA<MaterialX::Color3>();
                return color3Value[0] < 1.0 || color3Value[1] < 1.0 || color3Value[2] < 1.0;
            }
            catch(Exception)
            {
                return false;
            }
        }
    }
    return false;
}

} // namespace MaterialX
