//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenOsl/OslSyntax.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Nodes/SwizzleNode.h>
#include <MaterialXGenShader/Nodes/ConvertNode.h>
#include <MaterialXGenShader/Nodes/CombineNode.h>
#include <MaterialXGenShader/Nodes/SwitchNode.h>
#include <MaterialXGenShader/Nodes/IfNode.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>
#include <MaterialXGenShader/Nodes/ClosureAddNode.h>
#include <MaterialXGenShader/Nodes/ClosureMixNode.h>
#include <MaterialXGenShader/Nodes/ClosureMultiplyNode.h>
#include <MaterialXGenShader/Nodes/MaterialNode.h>

#include <MaterialXGenOsl/Nodes/BlurNodeOsl.h>
#include <MaterialXGenOsl/Nodes/SurfaceNodeOsl.h>
#include <MaterialXGenOsl/Nodes/ClosureLayerNodeOsl.h>

MATERIALX_NAMESPACE_BEGIN

const string OslShaderGenerator::TARGET = "genosl";
const string OslShaderGenerator::T_FILE_EXTRA_ARGUMENTS = "$extraTextureLookupArguments";

//
// OslShaderGenerator methods
//

OslShaderGenerator::OslShaderGenerator() :
    ShaderGenerator(OslSyntax::create())
{
    // Register build-in implementations

    // <!-- <if*> -->
    static const string SEPARATOR = "_";
    static const string INT_SEPARATOR = "I_";
    static const string BOOL_SEPARATOR = "B_";
    static const StringVec IMPL_PREFIXES = { "IM_ifgreater_", "IM_ifgreatereq_", "IM_ifequal_" };
    static const vector<CreatorFunction<ShaderNodeImpl>> IMPL_CREATE_FUNCTIONS =
            { IfGreaterNode::create,  IfGreaterEqNode::create, IfEqualNode::create };
    static const vector<bool> IMPL_HAS_INTVERSION = { true, true, true };
    static const vector<bool> IMPL_HAS_BOOLVERSION = { false, false, true };
    static const StringVec IMPL_TYPES = { "float", "color3", "color4", "vector2", "vector3", "vector4" };
    for (size_t i = 0; i<IMPL_PREFIXES.size(); i++)
    {
        const string& implPrefix = IMPL_PREFIXES[i];
        for (const string& implType : IMPL_TYPES)
        {
            const string implRoot = implPrefix + implType;
            registerImplementation(implRoot + SEPARATOR + OslShaderGenerator::TARGET, IMPL_CREATE_FUNCTIONS[i]);
            if (IMPL_HAS_INTVERSION[i])
            {
                registerImplementation(implRoot + INT_SEPARATOR + OslShaderGenerator::TARGET, IMPL_CREATE_FUNCTIONS[i]);
            }
            if (IMPL_HAS_BOOLVERSION[i])
            {
                registerImplementation(implRoot + BOOL_SEPARATOR + OslShaderGenerator::TARGET, IMPL_CREATE_FUNCTIONS[i]);
            }
        }
    }

    // <!-- <switch> -->
    // <!-- 'which' type : float -->
    registerImplementation("IM_switch_float_" + OslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_color3_" + OslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_color4_" + OslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_vector2_" + OslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_vector3_" + OslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_vector4_" + OslShaderGenerator::TARGET, SwitchNode::create);
    // <!-- 'which' type : integer -->
    registerImplementation("IM_switch_floatI_" + OslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_color3I_" + OslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_color4I_" + OslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_vector2I_" + OslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_vector3I_" + OslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_vector4I_" + OslShaderGenerator::TARGET, SwitchNode::create);

    // <!-- <swizzle> -->
    // <!-- from type : float -->
    registerImplementation("IM_swizzle_float_color3_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_color4_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector2_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector3_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector4_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : color3 -->
    registerImplementation("IM_swizzle_color3_float_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color3_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color4_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector2_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector3_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector4_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : color4 -->
    registerImplementation("IM_swizzle_color4_float_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color3_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color4_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector2_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector3_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector4_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : vector2 -->
    registerImplementation("IM_swizzle_vector2_float_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color3_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color4_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector2_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector3_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector4_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : vector3 -->
    registerImplementation("IM_swizzle_vector3_float_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color3_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color4_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector2_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector3_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector4_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : vector4 -->
    registerImplementation("IM_swizzle_vector4_float_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color3_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color4_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector2_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector3_" + OslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector4_" + OslShaderGenerator::TARGET, SwizzleNode::create);

    // <!-- <convert> -->
    registerImplementation("IM_convert_float_color3_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_float_color4_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_float_vector2_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_float_vector3_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_float_vector4_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector2_vector3_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector3_vector2_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector3_color3_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector3_vector4_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector4_vector3_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector4_color4_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_color3_vector3_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_color4_vector4_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_color3_color4_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_color4_color3_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_boolean_float_" + OslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_integer_float_" + OslShaderGenerator::TARGET, ConvertNode::create);

    // <!-- <combine> -->
    registerImplementation("IM_combine2_vector2_" + OslShaderGenerator::TARGET, CombineNode::create);
    registerImplementation("IM_combine2_color4CF_" + OslShaderGenerator::TARGET, CombineNode::create);
    registerImplementation("IM_combine2_vector4VF_" + OslShaderGenerator::TARGET, CombineNode::create);
    registerImplementation("IM_combine2_vector4VV_" + OslShaderGenerator::TARGET, CombineNode::create);
    registerImplementation("IM_combine3_color3_" + OslShaderGenerator::TARGET, CombineNode::create);
    registerImplementation("IM_combine3_vector3_" + OslShaderGenerator::TARGET, CombineNode::create);
    registerImplementation("IM_combine4_color4_" + OslShaderGenerator::TARGET, CombineNode::create);
    registerImplementation("IM_combine4_vector4_" + OslShaderGenerator::TARGET, CombineNode::create);

    // <!-- <blur> -->
    registerImplementation("IM_blur_float_" + OslShaderGenerator::TARGET, BlurNodeOsl::create);
    registerImplementation("IM_blur_color3_" + OslShaderGenerator::TARGET, BlurNodeOsl::create);
    registerImplementation("IM_blur_color4_" + OslShaderGenerator::TARGET, BlurNodeOsl::create);
    registerImplementation("IM_blur_vector2_" + OslShaderGenerator::TARGET, BlurNodeOsl::create);
    registerImplementation("IM_blur_vector3_" + OslShaderGenerator::TARGET, BlurNodeOsl::create);
    registerImplementation("IM_blur_vector4_" + OslShaderGenerator::TARGET, BlurNodeOsl::create);

    // <!-- <layer> -->
    registerImplementation("IM_layer_bsdf_" + OslShaderGenerator::TARGET, ClosureLayerNodeOsl::create);
    registerImplementation("IM_layer_vdf_" + OslShaderGenerator::TARGET, ClosureLayerNodeOsl::create);
    // <!-- <mix> -->
    registerImplementation("IM_mix_bsdf_" + OslShaderGenerator::TARGET, ClosureMixNode::create);
    registerImplementation("IM_mix_edf_" + OslShaderGenerator::TARGET, ClosureMixNode::create);
    // <!-- <add> -->
    registerImplementation("IM_add_bsdf_" + OslShaderGenerator::TARGET, ClosureAddNode::create);
    registerImplementation("IM_add_edf_" + OslShaderGenerator::TARGET, ClosureAddNode::create);
    // <!-- <multiply> -->
    registerImplementation("IM_multiply_bsdfC_" + OslShaderGenerator::TARGET, ClosureMultiplyNode::create);
    registerImplementation("IM_multiply_bsdfF_" + OslShaderGenerator::TARGET, ClosureMultiplyNode::create);
    registerImplementation("IM_multiply_edfC_" + OslShaderGenerator::TARGET, ClosureMultiplyNode::create);
    registerImplementation("IM_multiply_edfF_" + OslShaderGenerator::TARGET, ClosureMultiplyNode::create);

    // <!-- <thin_film> -->
    registerImplementation("IM_thin_film_bsdf_" + OslShaderGenerator::TARGET, NopNode::create);

    // <!-- <surface> -->
    registerImplementation("IM_surface_" + OslShaderGenerator::TARGET, SurfaceNodeOsl::create);

    // <!-- <surfacematerial> -->
    registerImplementation("IM_surfacematerial_" + OslShaderGenerator::TARGET, MaterialNode::create);

    // Extra arguments for texture lookups.
    _tokenSubstitutions[T_FILE_EXTRA_ARGUMENTS] = EMPTY_STRING;
}

ShaderPtr OslShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = createShader(name, element, context);

    ShaderGraph& graph = shader->getGraph();
    ShaderStage& stage = shader->getStage(Stage::PIXEL);

    emitLibraryIncludes(stage, context);

    // Add global constants and type definitions
    emitTypeDefinitions(context, stage);
    emitLine("#define M_FLOAT_EPS 1e-8", stage, false);
    emitLineBreak(stage);

    // Set the include file to use for uv transformations,
    // depending on the vertical flip flag.
    if (context.getOptions().fileTextureVerticalFlip)
    {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "mx_transform_uv_vflip.osl";
    }
    else
    {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "mx_transform_uv.osl";
    }

    // Emit function definitions for all nodes
    emitFunctionDefinitions(graph, context, stage);

    // Emit shader type, determined from the first
    // output if there are multiple outputs.
    const ShaderGraphOutputSocket* outputSocket0 = graph.getOutputSocket(0);
    if (outputSocket0->getType() == Type::SURFACESHADER)
    {
        emitString("surface ", stage);
    }
    else if (outputSocket0->getType() == Type::VOLUMESHADER)
    {
        emitString("volume ", stage);
    }
    else
    {
        emitString("shader ", stage);
    }

    // Begin shader signature. Note that makeIdentifier() will sanitize the name.
    string functionName = shader->getName();
    _syntax->makeIdentifier(functionName, graph.getIdentifierMap());
    setFunctionName(functionName, stage);
    emitLine(functionName, stage, false);

    const ShaderMetadataVecPtr& metadata = graph.getMetadata();
    bool haveShaderMetaData = metadata && metadata->size();

    // Always emit node information
    emitScopeBegin(stage, Syntax::DOUBLE_SQUARE_BRACKETS);
    emitLine("string mtlx_category = \"" + element->getCategory() + "\"" + Syntax::COMMA, stage, false);
    emitLine("string mtlx_name = \"" + element->getQualifiedName(element->getName())+ "\"" + 
            (haveShaderMetaData ? Syntax::COMMA : EMPTY_STRING), stage, false);

    // Add any metadata if set on the graph.
    if (haveShaderMetaData)
    {
        for (size_t j = 0; j < metadata->size(); ++j)
        {
            const ShaderMetadata& data = metadata->at(j);
            const string& delim = (j == metadata->size() - 1) ? EMPTY_STRING : Syntax::COMMA;
            const string& dataType = _syntax->getTypeName(data.type);
            const string dataValue = _syntax->getValue(data.type, *data.value, true);
            emitLine(dataType + " " + data.name + " = " + dataValue + delim, stage, false);
        }
    }
    emitScopeEnd(stage, false, false);
    emitLineEnd(stage, false);

    emitScopeBegin(stage, Syntax::PARENTHESES);

    // Emit shader inputs
    emitShaderInputs(stage.getInputBlock(OSL::INPUTS), stage);
    emitShaderInputs(stage.getUniformBlock(OSL::UNIFORMS), stage);

    // Emit shader output
    const VariableBlock& outputs = stage.getOutputBlock(OSL::OUTPUTS);
    emitShaderOutputs(outputs, stage);

    // End shader signature
    emitScopeEnd(stage);

    // Begin shader body
    emitFunctionBodyBegin(graph, context, stage);

    // Emit constants
    const VariableBlock& constants = stage.getConstantBlock();
    if (constants.size())
    {
        emitVariableDeclarations(constants, _syntax->getConstantQualifier(), Syntax::SEMICOLON, context, stage);
        emitLineBreak(stage);
    }

    // Emit all texturing nodes. These are inputs to any
    // closure/shader nodes and need to be emitted first.
    emitFunctionCalls(graph, context, stage, ShaderNode::Classification::TEXTURE);

    // Emit function calls for "root" closure/shader nodes.
    // These will internally emit function calls for any dependent closure nodes upstream.
    for (ShaderGraphOutputSocket* socket : graph.getOutputSockets())
    {
        if (socket->getConnection())
        {
            const ShaderNode* upstream = socket->getConnection()->getNode();
            if (upstream->getParent() == &graph &&
                (upstream->hasClassification(ShaderNode::Classification::CLOSURE) || 
                    upstream->hasClassification(ShaderNode::Classification::SHADER)))
            {
                emitFunctionCall(*upstream, context, stage);
            }
        }
    }

    // Emit final outputs
    for (size_t i = 0; i < outputs.size(); ++i)
    {
        const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket(i);
        const string result = getUpstreamResult(outputSocket, context);
        emitLine(outputSocket->getVariable() + " = " + result, stage);
    }

    // End shader body
    emitFunctionBodyEnd(graph, context, stage);

    // Perform token substitution
    replaceTokens(_tokenSubstitutions, stage);

    return shader;
}

void OslShaderGenerator::registerShaderMetadata(const DocumentPtr& doc, GenContext& context) const
{
    // Register all standard metadata.
    ShaderGenerator::registerShaderMetadata(doc, context);

    ShaderMetadataRegistryPtr registry = context.getUserData<ShaderMetadataRegistry>(ShaderMetadataRegistry::USER_DATA_NAME);
    if (!registry)
    {
        throw ExceptionShaderGenError("Registration of metadata faild");
    }

    // Rename the standard metadata names to corresponding OSL metadata names.
    const StringMap nameRemapping =
    {
        {ValueElement::UI_NAME_ATTRIBUTE, "label"},
        {ValueElement::UI_FOLDER_ATTRIBUTE, "page"},
        {ValueElement::UI_MIN_ATTRIBUTE, "min"},
        {ValueElement::UI_MAX_ATTRIBUTE, "max"},
        {ValueElement::UI_SOFT_MIN_ATTRIBUTE, "slidermin"},
        {ValueElement::UI_SOFT_MAX_ATTRIBUTE, "slidermax"},
        {ValueElement::UI_STEP_ATTRIBUTE, "sensitivity"},
        {ValueElement::DOC_ATTRIBUTE, "help"}
    };
    for (auto it : nameRemapping)
    {
        ShaderMetadata* data = registry->findMetadata(it.first);
        if (data)
        {
            data->name = it.second;
        }
    }
}

ShaderPtr OslShaderGenerator::createShader(const string& name, ElementPtr element, GenContext& context) const
{
    // Create the root shader graph
    ShaderGraphPtr graph = ShaderGraph::create(nullptr, name, element, context);
    ShaderPtr shader = std::make_shared<Shader>(name, graph);

    // Create our stage.
    ShaderStagePtr stage = createStage(Stage::PIXEL, *shader);
    stage->createUniformBlock(OSL::UNIFORMS);
    stage->createInputBlock(OSL::INPUTS);
    stage->createOutputBlock(OSL::OUTPUTS);

    // Create shader variables for all nodes that need this.
    for (ShaderNode* node : graph->getNodes())
    {
        node->getImplementation().createVariables(*node, context, *shader);
    }

    // Create uniforms for the published graph interface.
    VariableBlock& uniforms = stage->getUniformBlock(OSL::UNIFORMS);
    for (ShaderGraphInputSocket* inputSocket : graph->getInputSockets())
    {
        // Only for inputs that are connected/used internally,
        // and are editable by users.
        if (inputSocket->getConnections().size() && graph->isEditable(*inputSocket))
        {
            uniforms.add(inputSocket->getSelf());
        }
    }

    // Create outputs from the graph interface.
    VariableBlock& outputs = stage->getOutputBlock(OSL::OUTPUTS);
    for (ShaderGraphOutputSocket* outputSocket : graph->getOutputSockets())
    {
        outputs.add(outputSocket->getSelf());
    }

    return shader;
}

void OslShaderGenerator::emitFunctionCalls(const ShaderGraph& graph, GenContext& context, ShaderStage& stage, uint32_t classification) const
{
    // Special handling for closures functions.
    if ((classification & ShaderNode::Classification::CLOSURE) != 0)
    {
        // Emit function calls for closures connected to the outputs.
        // These will internally emit other closure function calls 
        // for upstream nodes if needed.
        for (ShaderGraphOutputSocket* outputSocket : graph.getOutputSockets())
        {
            const ShaderNode* upstream = outputSocket->getConnection() ? outputSocket->getConnection()->getNode() : nullptr;
            if (upstream && upstream->hasClassification(classification))
            {
                emitFunctionCall(*upstream, context, stage, false);
            }
        }
    }
    else
    {
        // Not a closures graph so just generate all
        // function calls in order.
        ShaderGenerator::emitFunctionCalls(graph, context, stage, classification);
    }
}

void OslShaderGenerator::emitFunctionBodyBegin(const ShaderNode& node, GenContext&, ShaderStage& stage, Syntax::Punctuation punc) const
{
    emitScopeBegin(stage, punc);

    if (node.hasClassification(ShaderNode::Classification::SHADER) || node.hasClassification(ShaderNode::Classification::CLOSURE))
    {
        emitLine("closure color null_closure = 0", stage);
    }
}

void OslShaderGenerator::emitLibraryIncludes(ShaderStage& stage, GenContext& context) const
{
    static const string INCLUDE_PREFIX = "#include \"";
    static const string INCLUDE_SUFFIX = "\"";
    static const StringVec INCLUDE_FILES =
    {
        "mx_funcs.h"
    };

    for (const string& file : INCLUDE_FILES)
    {
        FilePath path = context.resolveSourceFile(file, FilePath());

        // Force path to use slash since backslash even if escaped 
        // gives problems when saving the source code to file.
        string pathStr = path.asString();
        std::replace(pathStr.begin(), pathStr.end(), '\\', '/');

        emitLine(INCLUDE_PREFIX + pathStr + INCLUDE_SUFFIX, stage, false);
    }

    emitLineBreak(stage);
}

namespace
{
    std::unordered_map<string, string> GEOMPROP_DEFINITIONS =
    {
        {"Pobject", "transform(\"object\", P)"},
        {"Pworld", "P"},
        {"Nobject", "transform(\"object\", N)"},
        {"Nworld", "N"},
        {"Tobject", "transform(\"object\", dPdu)"},
        {"Tworld", "dPdu"},
        {"Bobject", "transform(\"object\", dPdv)"},
        {"Bworld", "dPdv"},
        {"UV0", "{u,v}"},
        {"Vworld", "I"}
    };
}

void OslShaderGenerator::emitShaderInputs(const VariableBlock& inputs, ShaderStage& stage) const
{
    const std::unordered_map<const TypeDesc*, ShaderMetadata> UI_WIDGET_METADATA =
    {
        { Type::FLOAT, ShaderMetadata("widget", Type::STRING, Value::createValueFromStrings("number", Type::STRING->getName())) },
        { Type::INTEGER, ShaderMetadata("widget", Type::STRING, Value::createValueFromStrings("number", Type::STRING->getName())) },
        { Type::FILENAME, ShaderMetadata("widget", Type::STRING, Value::createValueFromStrings("filename", Type::STRING->getName())) },
        { Type::BOOLEAN,  ShaderMetadata("widget", Type::STRING, Value::createValueFromStrings("checkBox", Type::STRING->getName())) }
    };

    const std::set<const TypeDesc*> METADATA_TYPE_BLACKLIST =
    {
        Type::VECTOR2, // Custom struct types doesn't support metadata declarations.
        Type::VECTOR4, //
        Type::COLOR4,  //
        Type::FILENAME, //
        Type::BSDF     //
    };

    for (size_t i = 0; i < inputs.size(); ++i)
    {
        const ShaderPort* input = inputs[i];

        const string& type = _syntax->getTypeName(input->getType());
        string value = _syntax->getValue((ShaderPort*)input, true);

        emitLineBegin(stage);
        emitString(type + " " + input->getVariable(), stage);

        const string& geomprop = input->getGeomProp();
        if (!geomprop.empty())
        {
            auto it = GEOMPROP_DEFINITIONS.find(geomprop);
            if (it != GEOMPROP_DEFINITIONS.end())
            {
                value = it->second;
            }
        }

        if (value.empty())
        {
            value = _syntax->getDefaultValue(input->getType());
        }
        emitString(" = " + value, stage);

        //
        // Add shader input metadata.
        //

        auto widgetMetadataIt = UI_WIDGET_METADATA.find(input->getType());
        const ShaderMetadata* widgetMetadata = widgetMetadataIt != UI_WIDGET_METADATA.end() ? &widgetMetadataIt->second : nullptr;
        const ShaderMetadataVecPtr& metadata = input->getMetadata();

        if (widgetMetadata || (metadata && metadata->size()))
        {
            StringVec metadataLines;
            if (metadata)
            {
                for (size_t j = 0; j < metadata->size(); ++j)
                {
                    const ShaderMetadata& data = metadata->at(j);
                    if (METADATA_TYPE_BLACKLIST.count(data.type) == 0)
                    {
                        const string& delim = (widgetMetadata || j < metadata->size() - 1) ? Syntax::COMMA : EMPTY_STRING;
                        const string& dataType = _syntax->getTypeName(data.type);
                        const string dataValue = _syntax->getValue(data.type, *data.value, true);
                        metadataLines.push_back(dataType + " " + data.name + " = " + dataValue + delim);
                    }
                }
            }
            if (widgetMetadata)
            {
                const string& dataType = _syntax->getTypeName(widgetMetadata->type);
                const string dataValue = _syntax->getValue(widgetMetadata->type, *widgetMetadata->value, true);
                metadataLines.push_back(dataType + " " + widgetMetadata->name + " = " + dataValue);
            }
            if (metadataLines.size())
            {
                emitLineEnd(stage, false);
                emitScopeBegin(stage, Syntax::DOUBLE_SQUARE_BRACKETS);
                for (auto line : metadataLines)
                {
                    emitLine(line, stage, false);
                }
                emitScopeEnd(stage, false, false);
            }
        }

        if (i < inputs.size())
        {
            emitString(",", stage);
        }

        emitLineEnd(stage, false);
    }
}

void OslShaderGenerator::emitShaderOutputs(const VariableBlock& outputs, ShaderStage& stage) const
{
    for (size_t i = 0; i < outputs.size(); ++i)
    {
        const ShaderPort* output = outputs[i];
        const TypeDesc* outputType = output->getType();
        const string type = _syntax->getOutputTypeName(outputType);
        const string value = _syntax->getDefaultValue(outputType, true);
        const string& delim = (i == outputs.size() - 1) ? EMPTY_STRING : Syntax::COMMA;
        emitLine(type + " " + output->getVariable() + " = " + value + delim, stage, false);
    }
}

namespace OSL
{
    // Identifiers for OSL variable blocks
    const string UNIFORMS = "u";
    const string INPUTS   = "i";
    const string OUTPUTS  = "o";
}

MATERIALX_NAMESPACE_END
