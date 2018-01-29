#include <MaterialXShaderGen/Implementations/Surface.h>
#include <MaterialXShaderGen/HwShader.h>
#include <MaterialXShaderGen/ShaderGenerators/GlslShaderGenerator.h>

namespace MaterialX
{

namespace
{
    static const string LIGHT_LOOP_BEGIN =
        "vec3 V = PS_IN.WorldView;\n"
        "const int numLights = min(ClampDynamicLights, 3);\n"
        "for (int ActiveLightIndex = 0; ActiveLightIndex < numLights; ++ActiveLightIndex)\n";

    static const string LIGHT_CONTRIBUTION =
        "vec3 LightPos = GetLightPos(ActiveLightIndex);\n"
        "vec3 LightDir = GetLightDir(ActiveLightIndex);\n"
        "vec3 LightVec = GetLightVectorFunction(ActiveLightIndex, LightPos, PS_IN.WorldPosition, LightDir);\n"
        "vec3 L = normalize(LightVec);\n"
        "vec3 LightContribution = LightContributionFunction(ActiveLightIndex, PS_IN.WorldPosition, LightVec);\n";
}

SgImplementationPtr SurfaceOgsFx::creator()
{
    return std::make_shared<SurfaceOgsFx>();
}

void SurfaceOgsFx::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader)
{
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

bool SurfaceOgsFx::isTransparent(const SgNode& node) const
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
