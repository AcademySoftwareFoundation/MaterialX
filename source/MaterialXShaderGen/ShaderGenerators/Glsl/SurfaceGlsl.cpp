#include <MaterialXShaderGen/ShaderGenerators/Glsl/SurfaceGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>

namespace MaterialX
{

namespace
{
    static const string LIGHT_LOOP_BEGIN =
        "vec3 V = vd.viewWorld;\n"
        "const int numLights = min(ClampDynamicLights, 3);\n"
        "for (int ActiveLightIndex = 0; ActiveLightIndex < numLights; ++ActiveLightIndex)\n";

    static const string LIGHT_CONTRIBUTION =
        "vec3 LightPos = GetLightPos(ActiveLightIndex);\n"
        "vec3 LightDir = GetLightDir(ActiveLightIndex);\n"
        "vec3 LightVec = GetLightVectorFunction(ActiveLightIndex, LightPos, vd.positionWorld, LightDir);\n"
        "vec3 L = normalize(LightVec);\n"
        "vec3 LightContribution = LightContributionFunction(ActiveLightIndex, vd.positionWorld, LightVec);\n";
}

SgImplementationPtr SurfaceGlsl::creator()
{
    return std::make_shared<SurfaceGlsl>();
}

void SurfaceGlsl::createVariables(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader_)
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

void SurfaceGlsl::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader_)
{
    HwShader& shader = static_cast<HwShader&>(shader_);

    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        const string& blockInstance = shader.getVertexDataBlock().instance;
        const string blockPrefix = blockInstance.length() ? blockInstance + "." : EMPTY_STRING;

        if (!shader.isCalculated("positionWorld"))
        {
            shader.setCalculated("positionWorld");
            shader.addLine(blockPrefix + "positionWorld = hPositionWorld.xyz");
        }
        if (!shader.isCalculated("viewWorld"))
        {
            shader.setCalculated("viewWorld");
            shader.addLine(blockPrefix + "viewWorld = normalize(u_viewInverseMatrix[3].xyz - hPositionWorld.xyz)");
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    GlslShaderGenerator& glslgen = static_cast<GlslShaderGenerator&>(shadergen);

    // Declare the output variable
    shader.beginLine();
    glslgen.emitOutput(node.getOutput(), true, shader);
    shader.endLine();
    shader.newLine();

    const string outputVariable = glslgen.getSyntax()->getVariableName(node.getOutput());
    const string outColor = outputVariable + ".color";
    const string outTransparency = outputVariable + ".transparency";

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
    shader.addLine(outColor + " += LightContribution * " + bsdf + ".fr");

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

    // Else case: return 100% transparency
    //
    shader.addLine("else", false);
    shader.beginScope();
    shader.addLine(outColor + " = vec3(0.0)");
    shader.addLine(outTransparency + " = vec3(1.0)");
    shader.endScope();
    shader.newLine();

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
