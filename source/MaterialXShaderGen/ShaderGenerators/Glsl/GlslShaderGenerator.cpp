#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslSyntax.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/PositionGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/NormalGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/TangentGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/BitangentGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/TexCoordGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GeomColorGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GeomAttrValueGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/FrameGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/TimeGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/SurfaceGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/SurfaceShaderGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/LightGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/LightCompoundGlsl.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/LightShaderGlsl.h>
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
}

const string GlslShaderGenerator::LANGUAGE = "sx-glsl";
const string GlslShaderGenerator::TARGET = "glsl_v4.0";
const string GlslShaderGenerator::VERSION = "400";
const string GlslShaderGenerator::LIGHT_DIR = "L";
const string GlslShaderGenerator::VIEW_DIR = "V";
const string GlslShaderGenerator::INCIDENT = "incident";
const string GlslShaderGenerator::OUTGOING = "outgoing";
const string GlslShaderGenerator::NORMAL = "normal";
const string GlslShaderGenerator::EVAL = "eval";

GlslShaderGenerator::GlslShaderGenerator()
    : ParentClass(GlslSyntax::create())
{
    // Direction vector argument for bsdf nodes
    _bsdfNodeArguments = 
    { 
        Argument("vec3", INCIDENT),
        Argument("vec3", OUTGOING)
    };

    // Direction vector argument for edf nodes
    _edfNodeArguments =
    {
        Argument("vec3", NORMAL),
        Argument("vec3", EVAL)
    };

    // <!-- <compare> -->
    registerImplementation("IM_compare__float__sx_glsl", Compare::create);
    registerImplementation("IM_compare__color2__sx_glsl", Compare::create);
    registerImplementation("IM_compare__color3__sx_glsl", Compare::create);
    registerImplementation("IM_compare__color4__sx_glsl", Compare::create);
    registerImplementation("IM_compare__vector2__sx_glsl", Compare::create);
    registerImplementation("IM_compare__vector3__sx_glsl", Compare::create);
    registerImplementation("IM_compare__vector4__sx_glsl", Compare::create);

    // <!-- <switch> -->
    registerImplementation("IM_switch__float__sx_glsl", Switch::create);
    registerImplementation("IM_switch__color2__sx_glsl", Switch::create);
    registerImplementation("IM_switch__color3__sx_glsl", Switch::create);
    registerImplementation("IM_switch__color4__sx_glsl", Switch::create);
    registerImplementation("IM_switch__vector2__sx_glsl", Switch::create);
    registerImplementation("IM_switch__vector3__sx_glsl", Switch::create);
    registerImplementation("IM_switch__vector4__sx_glsl", Switch::create);

    // <!-- <swizzle> -->
    // <!-- from type : float -->
    registerImplementation("IM_swizzle__float_color2__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__float_color3__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__float_color4__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__float_vector2__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__float_vector3__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__float_vector4__sx_glsl", Swizzle::create);
    // <!-- from type : color2 -->
    registerImplementation("IM_swizzle__color2_float__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color2_color2__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color2_color3__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color2_color4__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color2_vector2__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color2_vector3__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color2_vector4__sx_glsl", Swizzle::create);
    // <!-- from type : color3 -->
    registerImplementation("IM_swizzle__color3_float__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color3_color2__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color3_color3__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color3_color4__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color3_vector2__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color3_vector3__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color3_vector4__sx_glsl", Swizzle::create);
    // <!-- from type : color4 -->
    registerImplementation("IM_swizzle__color4_float__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color4_color2__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color4_color3__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color4_color4__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color4_vector2__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color4_vector3__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__color4_vector4__sx_glsl", Swizzle::create);
    // <!-- from type : vector2 -->
    registerImplementation("IM_swizzle__vector2_float__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector2_color2__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector2_color3__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector2_color4__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector2_vector2__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector2_vector3__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector2_vector4__sx_glsl", Swizzle::create);
    // <!-- from type : vector3 -->
    registerImplementation("IM_swizzle__vector3_float__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector3_color2__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector3_color3__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector3_color4__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector3_vector2__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector3_vector3__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector3_vector4__sx_glsl", Swizzle::create);
    // <!-- from type : vector4 -->
    registerImplementation("IM_swizzle__vector4_float__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector4_color2__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector4_color3__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector4_color4__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector4_vector2__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector4_vector3__sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle__vector4_vector4__sx_glsl", Swizzle::create);

    // <!-- <position> -->
    registerImplementation("IM_position__vector3__sx_glsl", PositionGlsl::create);
    // <!-- <normal> -->
    registerImplementation("IM_normal__vector3__sx_glsl", NormalGlsl::create);
    // <!-- <tangent> -->
    registerImplementation("IM_tangent__vector3__sx_glsl", TangentGlsl::create);
    // <!-- <bitangent> -->
    registerImplementation("IM_bitangent__vector3__sx_glsl", BitangentGlsl::create);
    // <!-- <texcoord> -->
    registerImplementation("IM_texcoord__vector2__sx_glsl", TexCoordGlsl::create);
    registerImplementation("IM_texcoord__vector3__sx_glsl", TexCoordGlsl::create);
    // <!-- <geomcolor> -->
    registerImplementation("IM_geomcolor__float__sx_glsl", GeomColorGlsl::create);
    registerImplementation("IM_geomcolor__color2__sx_glsl", GeomColorGlsl::create);
    registerImplementation("IM_geomcolor__color3__sx_glsl", GeomColorGlsl::create);
    registerImplementation("IM_geomcolor__color4__sx_glsl", GeomColorGlsl::create);
    // <!-- <geomattrvalue> -->
    registerImplementation("IM_geomattrvalue__integer__sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue__boolean__sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue__string__sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue__float__sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue__color2__sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue__color3__sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue__color4__sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue__vector2__sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue__vector3__sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue__vector4__sx_glsl", GeomAttrValueGlsl::create);

    // <!-- <frame> -->
    registerImplementation("IM_frame__float__sx_glsl", FrameGlsl::create);
    // <!-- <time> -->
    registerImplementation("IM_time__float__sx_glsl", TimeGlsl::create);

    // <!-- <surface> -->
    registerImplementation("IM_surface__sx_glsl", SurfaceGlsl::create);
    // <!-- <light> -->
    registerImplementation("IM_light__sx_glsl", LightGlsl::create);
    // <!-- <standardsurface> -->
    registerImplementation("IM_standardsurface__sx_glsl", SurfaceShaderGlsl::create);

    // <!-- <pointlight> -->
    registerImplementation("IM_pointlight__sx_glsl", LightShaderGlsl::create);
    // <!-- <directionallight> -->
    registerImplementation("IM_directionallight__sx_glsl", LightShaderGlsl::create);
    // <!-- <spotlight> -->
    registerImplementation("IM_spotlight__sx_glsl", LightShaderGlsl::create);
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
    shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, DataType::MATRIX4, "u_worldMatrix");
    shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, DataType::MATRIX4, "u_viewProjectionMatrix");

    // Add version directive
    shader.addLine("#version " + getVersion(), false);
    shader.newLine();

    // Add all private uniforms
    const Shader::VariableBlock& vsPrivateUniforms = shader.getUniformBlock(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS);
    if (vsPrivateUniforms.variableOrder.size())
    {
        shader.addComment("Uniform block: " + vsPrivateUniforms.name);
        for (const Shader::Variable* uniform : vsPrivateUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    // Add any public uniforms
    const Shader::VariableBlock& vsPublicUniforms = shader.getUniformBlock(HwShader::VERTEX_STAGE, HwShader::PUBLIC_UNIFORMS);
    if (vsPublicUniforms.variableOrder.size())
    {
        shader.addComment("Uniform block: " + vsPublicUniforms.name);
        for (const Shader::Variable* uniform : vsPublicUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    // Add all app data inputs
    const Shader::VariableBlock& appDataBlock = shader.getAppDataBlock();
    if (appDataBlock.variableOrder.size())
    {
        shader.addComment("Application data block: " + appDataBlock.name);
        for (const Shader::Variable* input : appDataBlock.variableOrder)
        {
            const string& type = _syntax->getTypeName(input->type);
            shader.addLine("in " + type + " " + input->name);
        }
        shader.newLine();
    }

    // Add vertex data block
    const Shader::VariableBlock& vertexDataBlock = shader.getVertexDataBlock();
    if (vertexDataBlock.variableOrder.size())
    {
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
    }

    emitFunctionDefinitions(shader);

    // Add main function
    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("vec4 hPositionWorld = u_worldMatrix * vec4(i_position, 1.0)");
    shader.addLine("gl_Position = u_viewProjectionMatrix * hPositionWorld");
    emitFunctionCalls(shader);
    shader.endScope();
    shader.newLine();

    //
    // Emit code for pixel shader stage
    //

    shader.setActiveStage(HwShader::PIXEL_STAGE);

    // Add version directive
    shader.addLine("#version " + getVersion(), false);
    shader.newLine();

    // Add global constants and type definitions
    shader.addInclude("sxpbrlib/sx-glsl/lib/sx_defines.glsl", *this);
    shader.addLine("#define MAX_LIGHT_SOURCES " + std::to_string(getMaxActiveLightSources()), false);
    shader.newLine();
    emitTypeDefs(shader);

    // Add all private uniforms
    const Shader::VariableBlock& psPrivateUniforms = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS);
    if (psPrivateUniforms.variableOrder.size())
    {
        shader.addComment("Uniform block: " + psPrivateUniforms.name);
        for (const Shader::Variable* uniform : psPrivateUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    // Add all public uniforms
    const Shader::VariableBlock& psPublicUniforms = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::PUBLIC_UNIFORMS);
    if (psPublicUniforms.variableOrder.size())
    {
        shader.addComment("Uniform block: " + psPublicUniforms.name);
        for (const Shader::Variable* uniform : psPublicUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    bool lighting = shader.hasClassification(SgNode::Classification::SHADER | SgNode::Classification::SURFACE);

    // Add light data block if needed
    if (lighting)
    {
        const Shader::VariableBlock& lightData = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::LIGHT_DATA_BLOCK);
        shader.addLine("struct " + lightData.name, false);
        shader.beginScope(Shader::Brackets::BRACES);
        for (const Shader::Variable* uniform : lightData.variableOrder)
        {
            const string& type = _syntax->getTypeName(uniform->type);
            shader.addLine(type + " " + uniform->name);
        }
        shader.endScope(true);
        shader.newLine();
        shader.addLine("uniform " + lightData.name + " " + lightData.instance + "[MAX_LIGHT_SOURCES]");
        shader.newLine();
    }

    // Add vertex data block
    if (vertexDataBlock.variableOrder.size())
    {
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
    }

    // Add the pixel shader output. This needs to be a vec4 for rendering
    // and upstream connection will be converted to vec4 if needed in emitFinalOutput()
    shader.addComment("Data output by the pixel shader");
    const SgOutputSocket* outputSocket = shader.getNodeGraph()->getOutputSocket();
    shader.addLine("out vec4 " + outputSocket->name);
    shader.newLine();

    // Emit common math functions
    shader.addInclude("sxpbrlib/sx-glsl/lib/sx_math.glsl", *this);
    shader.newLine();

    if (lighting)
    {
        // Emit lighting functions
        shader.addInclude("sxpbrlib/sx-glsl/lib/sx_lighting.glsl", *this);
        shader.newLine();
    }

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
    _bsdfNodeArguments[0].second = INCIDENT;
    _bsdfNodeArguments[1].second = OUTGOING;

    // Set EDF node arguments to the variables used for orientation and evaluation
    _edfNodeArguments[0].second = NORMAL;
    _edfNodeArguments[1].second = EVAL;

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

        // Emit function for handling texture coords v-flip 
        // as needed by the v-direction set by the user
        shader.addBlock(shader.getRequestedVDirection() != getTargetVDirection() ? VDIRECTION_FLIP : VDIRECTION_NOOP);

        // For surface shaders we need light shaders
        if (shader.hasClassification(SgNode::Classification::SHADER | SgNode::Classification::SURFACE))
        {
            // Emit functions for all bound light shaders
            for (auto lightShader : getBoundLightShaders())
            {
                lightShader.second->emitFunctionDefinition(SgNode::NONE, *this, shader);
            }

            // Emit active light count function
            shader.addLine("int numActiveLightSources()", false);
            shader.beginScope(Shader::Brackets::BRACES);
            shader.addLine("return min(u_numActiveLightSources, MAX_LIGHT_SOURCES)");
            shader.endScope();
            shader.newLine();

            // Emit light sampler function with all bound light types
            shader.addLine("void sampleLightSource(LightData light, vec3 position, out lightshader result)", false);
            shader.beginScope(Shader::Brackets::BRACES);
            shader.addLine("result.intensity = vec3(0.0)");
            string ifstatement = "if ";
            for (auto lightShader : getBoundLightShaders())
            {
                shader.addLine(ifstatement + "(light.type == " + std::to_string(lightShader.first) + ")", false);
                shader.beginScope(Shader::Brackets::BRACES);
                lightShader.second->emitFunctionCall(SgNode::NONE, *this, shader);
                shader.endScope();
                ifstatement = "else if ";
            }
            shader.endScope();
            shader.newLine();
        }

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Call parent to emit all other functions
    ParentClass::emitFunctionDefinitions(shader);
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
        if (shader.hasClassification(SgNode::Classification::SHADER | SgNode::Classification::SURFACE))
        {
            // Handle all texturing nodes. These are inputs to any
            // closure/shader nodes and need to be emitted first.
            emitTextureNodes(shader);

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
            ParentClass::emitFunctionCalls(shader);
        }
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

void GlslShaderGenerator::emitFinalOutput(Shader& shader) const
{
    const SgOutputSocket* outputSocket = shader.getNodeGraph()->getOutputSocket();

    // Early out for the rare case where the whole graph is just a single value
    if (!outputSocket->connection)
    {
        string outputValue = outputSocket->value ? 
            _syntax->getValue(*outputSocket->value, outputSocket->type) : 
            _syntax->getTypeDefault(outputSocket->type);
        if (!DataType::isQuadruple(outputSocket->type))
        {
            string finalOutput = outputSocket->name + "_tmp";
            shader.addLine(_syntax->getTypeName(outputSocket->type) + " " + finalOutput + " = " + outputValue);
            toVec4(outputSocket->type, finalOutput);
            shader.addLine(outputSocket->name + " = " + finalOutput);
        }
        else
        {
            shader.addLine(outputSocket->name + " = " + outputValue);
        }
        return;
    }

    string finalOutput = outputSocket->connection->name;

    if (shader.hasClassification(SgNode::Classification::SURFACE))
    {
        shader.addComment("TODO: How should we output transparency?");
        shader.addLine("float outAlpha = 1.0 - sx_max_component(" + finalOutput + ".transparency)");
        shader.addLine(outputSocket->name + " = vec4(" + finalOutput + ".color, outAlpha)");
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
        shader.addLine(outputSocket->name + " = " + finalOutput);
    }
}

void GlslShaderGenerator::emitTextureNodes(Shader& shader)
{
    // Emit function calls for all texturing nodes
    bool found = false;
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        if (node->hasClassification(SgNode::Classification::TEXTURE) && !node->referencedConditionally())
        {
            shader.addFunctionCall(node, *this);
            found = true;
        }
    }

    if (found)
    {
        shader.newLine();
    }
}

void GlslShaderGenerator::emitBsdfNodes(const SgNode& shaderNode, const string& incident, const string& outgoing, Shader& shader, string& bsdf)
{
    // Set BSDF node arguments according to the given directions
    _bsdfNodeArguments[0].second = incident;
    _bsdfNodeArguments[1].second = outgoing;

    SgNode* last = nullptr;

    // Emit function calls for all BSDF nodes used by this shader
    // The last node will hold the final result
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        if (node->hasClassification(SgNode::Classification::BSDF) && shaderNode.isUsedClosure(node))
        {
            shader.addFunctionCall(node, *this);
            last = node;
        }
    }

    if (last)
    {
        bsdf = last->getOutput()->name;
    }
}

void GlslShaderGenerator::emitEdfNodes(const SgNode& shaderNode, const string& orientDir, const string& evalDir, Shader& shader, string& edf)
{
    // Set EDF node arguments according to the given directions
    _edfNodeArguments[0].second = orientDir;
    _edfNodeArguments[1].second = evalDir;

    edf = "vec3(0.0)";

    SgNode* last = nullptr;

    // Emit function calls for all EDF nodes used by this shader
    // The last node will hold the final result
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        if (node->hasClassification(SgNode::Classification::EDF) && shaderNode.isUsedClosure(node))
        {
            shader.addFunctionCall(node, *this);
            last = node;
        }
    }

    if (last)
    {
        edf = last->getOutput()->name;
    }
}

const Arguments* GlslShaderGenerator::getExtraArguments(const SgNode& node) const
{
    if (node.hasClassification(SgNode::Classification::BSDF))
    {
        return &_bsdfNodeArguments;
    }
    else if (node.hasClassification(SgNode::Classification::EDF))
    {
        return &_edfNodeArguments;
    }
    return nullptr;
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
            line += " = " + _syntax->getValue(*uniform.value, uniform.type, true);
        else
            line += " = " + _syntax->getTypeDefault(uniform.type, true);
        shader.addLine(line);
    }
}

SgImplementationPtr GlslShaderGenerator::createCompoundImplementation(NodeGraphPtr impl)
{
    NodeDefPtr nodeDef = impl->getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Error creating compound implementation. Given nodegraph '" + impl->getName() + "' has no nodedef set");
    }
    if (nodeDef->getType() == DataType::LIGHT)
    {
        return LightCompoundGlsl::create();
    }
    return ParentClass::createCompoundImplementation(impl);
}


const string GlslImplementation::SPACE = "space";
const string GlslImplementation::WORLD = "world";
const string GlslImplementation::OBJECT = "object";
const string GlslImplementation::MODEL = "model";
const string GlslImplementation::INDEX = "index";
const string GlslImplementation::ATTRNAME = "attrname";

const string& GlslImplementation::getLanguage() const
{
    return GlslShaderGenerator::LANGUAGE;
}

const string& GlslImplementation::getTarget() const
{
    return GlslShaderGenerator::TARGET;
}

}
