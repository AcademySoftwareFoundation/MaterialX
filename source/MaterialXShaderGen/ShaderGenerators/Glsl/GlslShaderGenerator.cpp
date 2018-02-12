#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslSyntax.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/PositionGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/NormalGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/TangentGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/BitangentGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/TexCoordGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/AdskSurfaceGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/SurfaceGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/SourceCode.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/Swizzle.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/Switch.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/Compare.h>

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
const string GlslShaderGenerator::TARGET = "glsl_v4.0";

GlslShaderGenerator::GlslShaderGenerator()
    : ShaderGenerator(std::make_shared<GlslSyntax>())
{
    _bsdfNodeArguments.resize(2);

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

    // <!-- <position> -->
    registerImplementation("IM_position__vector3__glsl", PositionGlsl::creator);
    // <!-- <normal> -->
    registerImplementation("IM_normal__vector3__glsl", NormalGlsl::creator);
    // <!-- <tangent> -->
    registerImplementation("IM_tangent__vector3__glsl", TangentGlsl::creator);
    // <!-- <bitangent> -->
    registerImplementation("IM_bitangent__vector3__glsl", BitangentGlsl::creator);
    // <!-- <texcoord> -->
    registerImplementation("IM_texcoord__vector2__glsl", TexCoordGlsl::creator);
    registerImplementation("IM_texcoord__vector3__glsl", TexCoordGlsl::creator);

    // <!-- <adskSurface> -->
    registerImplementation("IM_adskSurface__glsl", AdskSurfaceGlsl::creator);
    // <!-- <surface> -->
    registerImplementation("IM_surface__glsl", SurfaceGlsl::creator);
}

ShaderPtr GlslShaderGenerator::generate(const string& shaderName, ElementPtr element)
{
    HwShaderPtr shaderPtr = std::make_shared<HwShader>(shaderName);
    shaderPtr->initialize(element, *this);

    HwShader& shader = *shaderPtr;

    //
    // Emit code for vertex shader stage
    //

    shader.setActiveStage(HwShader::VERTEX_STAGE);

    // Create required variables for vertex stage
    shader.createAppData(DataType::VECTOR3, "i_position");
    shader.createUniform(HwShader::GLOBAL_SCOPE, DataType::MATRIX4, "u_modelMatrix");
    shader.createUniform(HwShader::GLOBAL_SCOPE, DataType::MATRIX4, "u_viewProjectionMatrix");

    // Add version directive
    shader.addLine("#version 400", false);
    shader.newLine();

    // Add all global scope uniforms
    const Shader::VariableBlock& globalUniformBlock = shader.getUniformBlock(Shader::GLOBAL_SCOPE);
    for (const Shader::Variable* uniform : globalUniformBlock.variableOrder)
    {
        emitUniform(*uniform, shader);
    }
    shader.newLine();

    // Add all app data inputs
    const Shader::VariableBlock& appDataBlock = shader.getAppDataBlock();
    for (const Shader::Variable* input : appDataBlock.variableOrder)
    {
        const string& type = _syntax->getTypeName(input->type);
        shader.addLine("in " + type + " " + input->name);
    }
    shader.newLine();

    // Add vertex data block
    const Shader::VariableBlock& vertexDataBlock = shader.getVertexDataBlock();
    shader.addLine("out VertexData", false);
    shader.beginScope(Shader::Brackets::BRACES);
    for (const Shader::Variable* output : vertexDataBlock.variableOrder)
    {
        const string& type = _syntax->getTypeName(output->type);
        shader.addLine(type + " " + output->name);
    }
    shader.endScope(false, false);
    shader.addStr(" " + vertexDataBlock.instance + ";\n");
    shader.newLine();

    emitFunctionDefinitions(shader);

    // Add main function
    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("vec4 hPositionWorld = u_modelMatrix * vec4(i_position, 1.0)");
    shader.addLine("gl_Position = u_viewProjectionMatrix * hPositionWorld");
    emitFunctionCalls(shader);
    shader.endScope();
    shader.newLine();

    //
    // Emit code for pixel shader stage
    //

    shader.setActiveStage(HwShader::PIXEL_STAGE);

    // Add version directive
    shader.addLine("#version 460", false);
    shader.newLine();

    // Add global constants and type definitions
    shader.addInclude("sx/impl/shadergen/source/glsl/defines.glsl", *this);
    shader.newLine();
    emitTypeDefs(shader);

    // Add all global scope uniforms
    for (const Shader::Variable* uniform : globalUniformBlock.variableOrder)
    {
        emitUniform(*uniform, shader);
    }
    shader.newLine();

    // Add all shader interface uniforms
    const Shader::VariableBlock& shaderInterfaceBlock = shader.getUniformBlock(Shader::SHADER_INTERFACE);
    for (const Shader::Variable* uniform : shaderInterfaceBlock.variableOrder)
    {
        emitUniform(*uniform, shader);
    }
    shader.newLine();

    // Add vertex data block
    shader.addLine("in VertexData", false);
    shader.beginScope(Shader::Brackets::BRACES);
    for (const Shader::Variable* input : vertexDataBlock.variableOrder)
    {
        const string& type = _syntax->getTypeName(input->type);
        shader.addLine(type + " " + input->name);
    }
    shader.endScope(false, false);
    shader.addStr(" " + vertexDataBlock.instance + ";\n");
    shader.newLine();


    // Add the pixel shader output. This needs to be a vec4 for rendering
    // and upstream connection will be converted to vec4 if needed in emitFinalOutput()
    shader.addComment("Data output by the pixel shader");
    const SgOutputSocket* outputSocket = shader.getNodeGraph()->getOutputSocket();
    const string variable = _syntax->getVariableName(outputSocket);
    shader.addLine("out vec4 " + variable);
    shader.newLine();

    // Emit common math functions
    shader.addInclude("sx/impl/shadergen/source/glsl/math.glsl", *this);
    shader.newLine();

    // Add all functions for node implementations
    emitFunctionDefinitions(shader);

    // Add main function
    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);
    emitFunctionCalls(shader);
    emitFinalOutput(shader);
    shader.endScope();
    shader.newLine();

    return shaderPtr;
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

void GlslShaderGenerator::emitFinalOutput(Shader& shader) const
{
    const SgOutputSocket* outputSocket = shader.getNodeGraph()->getOutputSocket();
    const string& outputVariable = _syntax->getVariableName(outputSocket);

    // Early out for the rare case where the whole graph is just a single value
    if (!outputSocket->connection)
    {
        string outputValue = outputSocket->value ? _syntax->getValue(*outputSocket->value) : _syntax->getTypeDefault(outputSocket->type);
        if (!DataType::isQuadruple(outputSocket->type))
        {
            string finalOutput = outputVariable + "_tmp";
            shader.addLine(_syntax->getTypeName(outputSocket->type) + " " + finalOutput + " = " + outputValue);
            toVec4(outputSocket->type, finalOutput);
            shader.addLine(outputVariable + " = " + finalOutput);
        }
        else
        {
            shader.addLine(outputVariable + " = " + outputValue);
        }
        return;
    }

    string finalOutput = _syntax->getVariableName(outputSocket->connection);

    if (shader.hasClassification(SgNode::Classification::SURFACE))
    {
        shader.addComment("TODO: How should we output transparency?");
        shader.addLine("float outAlpha = 1.0 - maxv(" + finalOutput + ".transparency)");
        shader.addLine(outputVariable + " = vec4(" + finalOutput + ".color, outAlpha)");
    }
    else
    {
        if (outputSocket->channels != EMPTY_STRING)
        {
            finalOutput = _syntax->getSwizzledVariable(finalOutput, outputSocket->type, outputSocket->connection->type, outputSocket->channels);
        }
        if (!DataType::isQuadruple(outputSocket->type))
        {
            toVec4(outputSocket->type, finalOutput);
        }
        shader.addLine(outputVariable + " = " + finalOutput);
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

void GlslShaderGenerator::toVec4(const string& type, string& variable)
{
    if (DataType::isScalar(type))
    {
        variable = "vec4(" + variable + ", " + variable + ", " + variable + ", 1.0)";
    }
    else if (DataType::isTuple(type))
    {
        variable = "vec4(" + variable + ", 0.0, 1.0)";
    }
    else if (DataType::isTriple(type))
    {
        variable = "vec4(" + variable + ", 1.0)";
    }
    else
    {
        // Can't understand other types. Just return black.
        variable = "vec4(0.0,0.0,0.0,1.0)";
    }
}

void GlslShaderGenerator::emitUniform(const Shader::Variable& uniform, Shader& shader)
{
    // A file texture input needs special handling on GLSL
    if (uniform.type == DataType::FILENAME)
    {
        shader.addLine("uniform sampler2D " + uniform.name);
    }
    else
    {
        const string& type = _syntax->getTypeName(uniform.type);
        string line = "uniform " + type + " " + uniform.name;
        if (uniform.semantic.length())
            line += " : " + uniform.semantic;
        if (uniform.value)
            line += " = " + _syntax->getValue(*uniform.value, true);
        else
            line += " = " + _syntax->getTypeDefault(uniform.type, true);
        shader.addLine(line);
    }
}


const string GlslImplementation::SPACE = "space";
const string GlslImplementation::WORLD = "world";
const string GlslImplementation::OBJECT = "object";
const string GlslImplementation::MODEL = "model";
const string GlslImplementation::INDEX = "index";

const string& GlslImplementation::getLanguage() const
{
    return GlslShaderGenerator::LANGUAGE;
}

const string& GlslImplementation::getTarget() const
{
    return GlslShaderGenerator::TARGET;
}

}
