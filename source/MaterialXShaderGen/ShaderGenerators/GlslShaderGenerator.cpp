#include <MaterialXShaderGen/ShaderGenerators/GlslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/GlslSyntax.h>

#include <MaterialXShaderGen/Implementations/SourceCode.h>
#include <MaterialXShaderGen/Implementations/Swizzle.h>
#include <MaterialXShaderGen/Implementations/Switch.h>
#include <MaterialXShaderGen/Implementations/Compare.h>

#include <MaterialXShaderGen/HwShader.h>

namespace MaterialX
{

namespace
{
    const char* VDIRECTION_FLIP =
        "void vdirection(vec2 texcoord, out vec2 result)\n"
        "{\n"
        "   result.x = texcoord.x;\n"
        "   result.y = 1.0 - texcoord.y;\n"
        "}\n\n";

    const char* VDIRECTION_NOOP =
        "void vdirection(vec2 texcoord, out vec2 result)\n"
        "{\n"
        "   result = texcoord;\n"
        "}\n\n";

    // Arguments used to represent BSDF direction vectors
    Arguments BSDF_DIR_ARGUMENTS =
    {
        Argument("vec3", "L"), // BsdfDir::LIGHT_DIR
        Argument("vec3", "V"), // BsdfDir::VIEW_DIR
        Argument("vec3", "R")  // BsdfDir::REFL_DIR
    };
}

const string GlslShaderGenerator::LANGUAGE = "glsl";

GlslShaderGenerator::GlslShaderGenerator()
    : ShaderGenerator(std::make_shared<GlslSyntax>())
{
    _bsdfNodeArguments.resize(2);

    // Add target specific implementations

    // <!-- <compare> -->
    registerImplementation("IM_compare__float__glsl", Compare::creator);
    registerImplementation("IM_compare__color2__glsl", Compare::creator);
    registerImplementation("IM_compare__color3__glsl", Compare::creator);
    registerImplementation("IM_compare__color4__glsl", Compare::creator);
    registerImplementation("IM_compare__vector2__glsl", Compare::creator);
    registerImplementation("IM_compare__vector3__glsl", Compare::creator);
    registerImplementation("IM_compare__vector4__glsl", Compare::creator);

    // <!-- <switch> -->
    registerImplementation("IM_switch__float__glsl", Switch::creator);
    registerImplementation("IM_switch__color2__glsl", Switch::creator);
    registerImplementation("IM_switch__color3__glsl", Switch::creator);
    registerImplementation("IM_switch__color4__glsl", Switch::creator);
    registerImplementation("IM_switch__vector2__glsl", Switch::creator);
    registerImplementation("IM_switch__vector3__glsl", Switch::creator);
    registerImplementation("IM_switch__vector4__glsl", Switch::creator);

    // <!-- <swizzle> -->
    // <!-- from type : float -->
    registerImplementation("IM_swizzle__float_color2__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__float_color3__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__float_color4__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__float_vector2__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__float_vector3__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__float_vector4__glsl", Swizzle::creator);
    // <!-- from type : color2 -->
    registerImplementation("IM_swizzle__color2_float__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color2_color2__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color2_color3__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color2_color4__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color2_vector2__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color2_vector3__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color2_vector4__glsl", Swizzle::creator);
    // <!-- from type : color3 -->
    registerImplementation("IM_swizzle__color3_float__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color3_color2__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color3_color3__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color3_color4__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color3_vector2__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color3_vector3__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color3_vector4__glsl", Swizzle::creator);
    // <!-- from type : color4 -->
    registerImplementation("IM_swizzle__color4_float__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color4_color2__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color4_color3__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color4_color4__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color4_vector2__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color4_vector3__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__color4_vector4__glsl", Swizzle::creator);
    // <!-- from type : vector2 -->
    registerImplementation("IM_swizzle__vector2_float__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector2_color2__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector2_color3__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector2_color4__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector2_vector2__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector2_vector3__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector2_vector4__glsl", Swizzle::creator);
    // <!-- from type : vector3 -->
    registerImplementation("IM_swizzle__vector3_float__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector3_color2__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector3_color3__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector3_color4__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector3_vector2__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector3_vector3__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector3_vector4__glsl", Swizzle::creator);
    // <!-- from type : vector4 -->
    registerImplementation("IM_swizzle__vector4_float__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector4_color2__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector4_color3__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector4_color4__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector4_vector2__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector4_vector3__glsl", Swizzle::creator);
    registerImplementation("IM_swizzle__vector4_vector4__glsl", Swizzle::creator);
}

void GlslShaderGenerator::emitFunctionDefinitions(Shader& shader)
{
    // Set BSDF node arguments to the variables used for BSDF direction vectors
    _bsdfNodeArguments[0] = Argument("vec3", "wi");
    _bsdfNodeArguments[1] = Argument("vec3", "wo");

    // Emit function for handling texture coords v-flip 
    // as needed by the v-direction set by the user
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        shader.addBlock(shader.getRequestedVDirection() != getTargetVDirection() ? VDIRECTION_FLIP : VDIRECTION_NOOP);
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Call parent to emit all other functions
    ShaderGenerator::emitFunctionDefinitions(shader);
}

void GlslShaderGenerator::emitFunctionCalls(Shader &shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        // For vertex stage just emit all function calls in order
        // and ignore conditional scope.
        for (SgNode* node : shader.getNodeGraph()->getNodes())
        {
            shader.addFunctionCall(node, *this);
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        // For pixel stage surface shaders need special handling
        if (shader.getNodeGraph()->hasClassification(SgNode::Classification::SHADER | SgNode::Classification::SURFACE))
        {
            // Handle all texturing nodes. These are inputs to any
            // closure/shader nodes and need to be emitted first.
            emitTextureNodes(shader);
            shader.newLine();

            // Emit function calls for all surface shader nodes
            for (SgNode* node : shader.getNodeGraph()->getNodes())
            {
                if (node->hasClassification(SgNode::Classification::SHADER | SgNode::Classification::SURFACE))
                {
                    shader.addFunctionCall(node, *this);
                }
            }
        }
        else
        {
            // No surface shader, fallback to base class
            ShaderGenerator::emitFunctionCalls(shader);
        }
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

void GlslShaderGenerator::emitUniform(const Shader::Variable& uniform, Shader& shader)
{
    // A file texture input needs special handling on GLSL
    if (uniform.type == DataType::FILENAME)
    {
        std::stringstream str;
        str << "uniform texture2D " << uniform.name << "_texture : SourceTexture;\n";
        str << "uniform sampler2D " << uniform.name << " = sampler_state\n";
        str << "{\n    Texture = <" << uniform.name << "_texture>;\n};\n";
        shader.addBlock(str.str());
    }
    else if (!uniform.semantic.empty())
    {
        shader.addLine("uniform " + _syntax->getTypeName(uniform.type) + " " + uniform.name + " : " + uniform.semantic);
    }
    else
    {
        const string initStr = (uniform.value ? _syntax->getValue(*uniform.value, true) : _syntax->getTypeDefault(uniform.type, true));
        shader.addLine("uniform " + _syntax->getTypeName(uniform.type) + " " + uniform.name + (initStr.empty() ? "" : " = " + initStr));
    }
}

bool GlslShaderGenerator::shouldPublish(const ValueElement* port, string& publicName) const
{
    if (!ShaderGenerator::shouldPublish(port, publicName))
    {
        // File texture inputs must be published in GLSL
        static const string IMAGE_NODE_NAME = "image";
        if (port->getParent()->getCategory() == IMAGE_NODE_NAME &&
            port->getType() == DataType::FILENAME)
        {
            publicName = port->getParent()->getName() + "_" + port->getName();
            return true;
        }
        else
        {
            return false;
        }
    }
    return true;
}

void GlslShaderGenerator::emitTextureNodes(Shader& shader)
{
    // Emit function calls for all texturing nodes
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        if (node->hasClassification(SgNode::Classification::TEXTURE) && !node->referencedConditionally())
        {
            shader.addFunctionCall(node, *this);
        }
    }
}

void GlslShaderGenerator::emitSurfaceBsdf(const SgNode& surfaceShaderNode, BsdfDir wi, BsdfDir wo, Shader& shader, string& bsdf)
{
    // Set BSDF node arguments according to the given directions
    _bsdfNodeArguments[0] = BSDF_DIR_ARGUMENTS[size_t(wi)];
    _bsdfNodeArguments[1] = BSDF_DIR_ARGUMENTS[size_t(wo)];

    SgNode* last = nullptr;

    // Emit function calls for all BSDF nodes used by this shader
    // The last node will hold the final result
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        if (node->hasClassification(SgNode::Classification::BSDF) && surfaceShaderNode.isUsedClosure(node))
        {
            shader.addFunctionCall(node, *this);
            last = node;
        }
    }

    if (last)
    {
        bsdf = _syntax->getVariableName(last->getOutput());
    }
}

void GlslShaderGenerator::emitSurfaceEmission(const SgNode& surfaceShaderNode, Shader& shader, string& emission)
{
    emission = "vec3(0.0)";

    SgNode* last = nullptr;

    // Emit function calls for all EDF nodes used by this shader
    // The last node will hold the final result
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        if (node->hasClassification(SgNode::Classification::EDF) && surfaceShaderNode.isUsedClosure(node))
        {
            shader.addFunctionCall(node, *this);
            last = node;
        }
    }

    if (last)
    {
        emission = _syntax->getVariableName(last->getOutput());
    }
}

const Arguments* GlslShaderGenerator::getExtraArguments(const SgNode& node) const
{
    return node.hasClassification(SgNode::Classification::BSDF) ? &_bsdfNodeArguments : nullptr;
}

}
