#include <MaterialXGenGlsl/Nodes/SurfaceShaderGlsl.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

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
    // The surface shader needs position, view position and light sources. We should solve this by adding some 
    // dependency mechanism so this implementation can be set to depend on the PositionGlsl,  
    // ViewDirectionGlsl and LightGlsl nodes instead? This is where the MaterialX attribute "internalgeomprops" 
    // is needed.
    //
    HwShader& shader = static_cast<HwShader&>(shader_);

    shader.createAppData(Type::VECTOR3, "i_position");
    shader.createVertexData(Type::VECTOR3, "positionWorld");
    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, Type::VECTOR3, "u_viewPosition");
    shader.createUniform(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS, Type::INTEGER, "u_numActiveLightSources",
        EMPTY_STRING, Value::createValue<int>(0));
}

void SurfaceShaderGlsl::emitFunctionCall(const SgNode& node, const SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader_)
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
        SourceCode::emitFunctionCall(node, context, shadergen, shader_);
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
