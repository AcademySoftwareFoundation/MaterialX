#include <MaterialXShaderGen/ShaderGenerators/Glsl/AdskSurfaceGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>

namespace MaterialX
{

SgImplementationPtr AdskSurfaceGlsl::creator()
{
    return std::make_shared<AdskSurfaceGlsl>();
}

const string& AdskSurfaceGlsl::getLanguage() const
{
    return GlslShaderGenerator::LANGUAGE;
}

const string& AdskSurfaceGlsl::getTarget() const
{
    return GlslShaderGenerator::TARGET;
}


void AdskSurfaceGlsl::createVariables(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
{
    // TODO: 
    // The surface shader needs position and view data. We should solve this by adding some 
    // dependency mechanism so this implementation can be set to depend on the PositionGlsl 
    // and ViewGlsl nodes instead? This is where the MaterialX attribute "internalgeomprops" 
    // is needed.
    //
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createAppData(DataType::VECTOR3, "i_position");

    shader.createUniform(HwShader::GLOBAL_SCOPE, DataType::MATRIX4, "u_viewInverseMatrix");

    shader.createVertexData(DataType::VECTOR3, "positionWorld");
    shader.createVertexData(DataType::VECTOR3, "viewWorld");
}

void AdskSurfaceGlsl::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (!shader.isCalculated("positionWorld"))
        {
            shader.setCalculated("positionWorld");
            shader.addLine("vd.positionWorld = hPositionWorld.xyz");
        }
        if (!shader.isCalculated("viewWorld"))
        {
            shader.setCalculated("viewWorld");
            shader.addLine("vd.viewWorld = normalize(u_viewInverseMatrix[3].xyz - hPositionWorld.xyz)");
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        SourceCode::emitFunctionCall(node, shadergen, shader_);
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

bool AdskSurfaceGlsl::isTransparent(const SgNode& node) const
{
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
