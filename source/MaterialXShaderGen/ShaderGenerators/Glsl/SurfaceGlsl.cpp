#include <MaterialXShaderGen/ShaderGenerators/Glsl/SurfaceGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>

namespace MaterialX
{

namespace
{
    static const string LIGHT_LOOP_BEGIN =
        "vec3 V = u_viewDirection;\n"
        "int numLights = numActiveLightSources();\n"
        "lightshader lightShader;\n"
        "for (int activeLightIndex = 0; activeLightIndex < numLights; ++activeLightIndex)\n";

    static const string LIGHT_CONTRIBUTION =
        "sampleLightSource(u_lightData[activeLightIndex], vd.positionWorld, lightShader);\n"
        "vec3 L = lightShader.direction;\n";
}

SgImplementationPtr SurfaceGlsl::creator()
{
    return std::make_shared<SurfaceGlsl>();
}

void SurfaceGlsl::createVariables(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
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
        EMPTY_STRING, Value::createValue<int>(1));
}

void SurfaceGlsl::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        if (!shader.isCalculated("positionWorld"))
        {
            const string& blockInstance = shader.getVertexDataBlock().instance;
            const string blockPrefix = blockInstance.length() ? blockInstance + "." : EMPTY_STRING;
            shader.addLine(blockPrefix + "positionWorld = hPositionWorld.xyz");
            shader.setCalculated("positionWorld");
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    GlslShaderGenerator& glslgen = static_cast<GlslShaderGenerator&>(shadergen);

    // Declare the output variable
    shader.beginLine();
    glslgen.emitOutput(node.getOutput(), true, shader);
    shader.endLine();

    const string outputVariable = glslgen.getSyntax()->getVariableName(node.getOutput());
    const string outColor = outputVariable + ".color";
    const string outTransparency = outputVariable + ".transparency";

    shader.addLine(outColor + " = vec3(0.0)");
    shader.addLine(outTransparency + " = vec3(1.0)");
    shader.newLine();

    // Calculate opacity
    //
    shader.beginLine();

    // Check for 100% transparency
    //
    string surfaceOpacity = node.getName() + "_opacity";
    shader.addStr("float " + surfaceOpacity + " = ");
    glslgen.emitInput(node.getInput("opacity"), shader);

    shader.endLine();
    shader.addLine("if (" + surfaceOpacity + " > 0.001)", false);
    shader.beginScope();

    // Handle direct lighting
    //
    shader.addComment("Light loop");
    shader.addBlock(LIGHT_LOOP_BEGIN);
    shader.beginScope();

    shader.addBlock(LIGHT_CONTRIBUTION);
    shader.newLine();

    shader.addComment("Calculate the BSDF response for this light source");
    string bsdf;
    glslgen.emitSurfaceBsdf(node, GlslShaderGenerator::BsdfDir::LIGHT_DIR, GlslShaderGenerator::BsdfDir::VIEW_DIR, shader, bsdf);
    shader.newLine();

    shader.addComment("Accumulate the light's contribution");
    shader.addLine(outColor + " += lightShader.intensity * " + bsdf + ".fr");

    shader.endScope();
    shader.newLine();

    //
    // TODO: Handle indirect lighting. Implement environment lighting for diffuse & specular BSDF's
    //

    // Handle surface emission
    //
    shader.addComment("Add surface emission");
    shader.beginScope();
    string emission;
    glslgen.emitSurfaceEmission(node, shader, emission);
    shader.addLine(outColor + " += " + emission);
    shader.endScope();
    shader.newLine();

    // Handle surface transparency
    //
    shader.addComment("Calculate the BSDF transmission for viewing direction");
    shader.beginScope();
    glslgen.emitSurfaceBsdf(node, GlslShaderGenerator::BsdfDir::VIEW_DIR, GlslShaderGenerator::BsdfDir::VIEW_DIR, shader, bsdf);
    shader.addLine(outTransparency + " = " + bsdf + ".ft");
    shader.endScope();
    shader.newLine();

    // Mix in opacity which affect the total result
    //
    shader.addLine(outColor + " *= " + surfaceOpacity);
    shader.addLine(outTransparency + " = mix(vec3(1.0), " + outTransparency + ", " + surfaceOpacity + ")");

    shader.endScope();

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

bool SurfaceGlsl::isTransparent(const SgNode& node) const
{
    if (node.getInput("opacity"))
    {
        MaterialX::ValuePtr value = node.getInput("opacity")->value;
        if (value)
        {
            try
            {
                float opacityValue = value->asA<float>();
                return (opacityValue < 1.0);
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
