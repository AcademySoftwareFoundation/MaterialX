#include <MaterialXShaderGen/ShaderGenerators/GlslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/GlslSyntax.h>

#include <MaterialXShaderGen/Implementations/Swizzle.h>
#include <MaterialXShaderGen/Implementations/Switch.h>
#include <MaterialXShaderGen/Implementations/Compare.h>

namespace
{
    const char* kVDirectionFlip =
        "void vdirection(vec2 texcoord, out vec2 result)\n"
        "{\n"
        "   result.x = texcoord.x;\n"
        "   result.y = 1.0 - texcoord.y;\n"
        "}\n\n";

    const char* kVDirectionNoop =
        "void vdirection(vec2 texcoord, out vec2 result)\n"
        "{\n"
        "   result = texcoord;\n"
        "}\n\n";
}

namespace MaterialX
{

GlslShaderGenerator::GlslShaderGenerator()
    : ShaderGenerator(std::make_shared<GlslSyntax>())
{
    _bsdfNodeArguments.resize(2);

    _bsdfDirArguments.resize(3);
    _bsdfDirArguments[size_t(BsdfDir::LIGHT_DIR)] = Argument("vec3", "L");
    _bsdfDirArguments[size_t(BsdfDir::VIEW_DIR)]  = Argument("vec3", "V");
    _bsdfDirArguments[size_t(BsdfDir::REFL_DIR)]  = Argument("vec3", "R");

    // Register build-in node implementations

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

void GlslShaderGenerator::emitFunctions(Shader& shader)
{
    // Emit function for handling texture coords v-flip 
    // as needed by the v-direction set by the user
    shader.addBlock(shader.getRequestedVDirection() != getTargetVDirection() ? kVDirectionFlip : kVDirectionNoop);

    _bsdfNodeArguments[0] = Argument("vec3", "wi");
    _bsdfNodeArguments[1] = Argument("vec3", "wo");
    
    // Call parent to emit all other functions
    ShaderGenerator::emitFunctions(shader);
}

void GlslShaderGenerator::emitTextureNodes(Shader& shader)
{
    // Emit function calls for all texturing nodes
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        // Emit only unconditional nodes, since any node within a conditional 
        // branch is emitted by the conditional node itself
        if (node->hasClassification(SgNode::Classification::TEXTURE) && !node->referencedConditionally())
        {
            shader.addFunctionCall(node, *this);
        }
    }
}

void GlslShaderGenerator::emitSurfaceBsdf(const SgNode& surfaceShaderNode, BsdfDir wi, BsdfDir wo, Shader& shader, string& bsdf)
{
    _bsdfNodeArguments[0] = _bsdfDirArguments[size_t(wi)];
    _bsdfNodeArguments[1] = _bsdfDirArguments[size_t(wo)];

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

const vector<ShaderGenerator::Argument>* GlslShaderGenerator::getExtraArguments(const SgNode& node) const
{
    return node.hasClassification(SgNode::Classification::BSDF) ? &_bsdfNodeArguments : nullptr;
}

}
