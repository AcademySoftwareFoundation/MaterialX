//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenOsl/OslSyntax.h>

#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/TypeDesc.h>

MATERIALX_NAMESPACE_BEGIN

const string OslShaderGenerator::TARGET = "genosl";

//
// OslShaderGenerator methods
//

OslShaderGenerator::OslShaderGenerator(TypeSystemPtr typeSystem) :
    ShaderGenerator(typeSystem, OslSyntax::create(typeSystem))
{
}

ShaderPtr OslShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = createShader(name, element, context);

    // Request fixed floating-point notation for consistency across targets.
    ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    ShaderGraph& graph = shader->getGraph();

    if (context.getOptions().oslConnectCiWrapper)
    {
        addSetCiTerminalNode(graph, element->getDocument(), context);
    }

    ShaderStage& stage = shader->getStage(Stage::PIXEL);

    emitLibraryIncludes(stage, context);

    // Add global constants and type definitions
    emitTypeDefinitions(context, stage);
    emitLine("#define M_FLOAT_EPS 1e-8", stage, false);
    emitLine("closure color null_closure() { closure color null_closure = 0; return null_closure; } ", stage, false);
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
    emitLine("string mtlx_name = \"" + element->getQualifiedName(element->getName()) + "\"" +
                 (haveShaderMetaData ? Syntax::COMMA : EMPTY_STRING),
             stage, false);

    // Add any metadata if set on the graph.
    if (haveShaderMetaData)
    {
        for (size_t j = 0; j < metadata->size(); ++j)
        {
            const ShaderMetadata& data = (*metadata)[j];
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

    // Emit shader outputs
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

    // Inputs of type 'filename' has been generated into two shader inputs.
    // So here we construct a single 'textureresource' from these inputs,
    // to be used further downstream. See emitShaderInputs() for details.
    VariableBlock& inputs = stage.getUniformBlock(OSL::UNIFORMS);
    for (size_t i = 0; i < inputs.size(); ++i)
    {
        ShaderPort* input = inputs[i];
        if (input->getType() == Type::FILENAME)
        {
            // Construct the textureresource variable.
            const string newVariableName = input->getVariable() + "_";
            const string& type = _syntax->getTypeName(input->getType());
            emitLine(type + newVariableName + " = {" + input->getVariable() + ", " + input->getVariable() + "_colorspace}", stage);

            // Update the variable name to be used downstream.
            input->setVariable(newVariableName);
        }
    }

    // Emit function calls for all nodes in the graph, starting each output
    // port and walking backwards along the incoming connections
    for (ShaderGraphOutputSocket* socket : graph.getOutputSockets())
    {
        if (socket->getConnection())
        {
            const ShaderNode* upstream = socket->getConnection()->getNode();
            if (upstream->getParent() == &graph)
            {
                emitAllDependentFunctionCalls(*upstream, context, stage);
            }
        }
    }

    // Assign results to final outputs.
    for (size_t i = 0; i < outputs.size(); ++i)
    {
        const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket(i);
        emitLine(outputSocket->getVariable() + " = " + getUpstreamResult(outputSocket, context), stage);
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
        throw ExceptionShaderGenError("Registration of metadata failed");
    }

    // Rename the standard metadata names to corresponding OSL metadata names.
    const StringMap nameRemapping =
    {
        { ValueElement::UI_NAME_ATTRIBUTE, "label" },
        { ValueElement::UI_FOLDER_ATTRIBUTE, "page" },
        { ValueElement::UI_MIN_ATTRIBUTE, "min" },
        { ValueElement::UI_MAX_ATTRIBUTE, "max" },
        { ValueElement::UI_SOFT_MIN_ATTRIBUTE, "slidermin" },
        { ValueElement::UI_SOFT_MAX_ATTRIBUTE, "slidermax" },
        { ValueElement::UI_STEP_ATTRIBUTE, "sensitivity" },
        { ValueElement::DOC_ATTRIBUTE, "help" }
    };
    for (const auto& it : nameRemapping)
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

    // Special handling for surfaceshader type output - if we have a material
    // that outputs a single surfaceshader then we will implicitly add a surfacematerial
    // node to create the final closure color - the surfaceshader type is a struct and needs
    // flattening to a single closure in the surfacematerial node.
    const auto& outputSockets = graph->getOutputSockets();
    const auto* singleOutput = outputSockets.size() == 1 ? outputSockets[0] : NULL;

    const bool isSurfaceShaderOutput = context.getOptions().oslImplicitSurfaceShaderConversion && singleOutput && singleOutput->getType() == Type::SURFACESHADER;

    if (isSurfaceShaderOutput)
    {
        graph->inlineNodeBeforeOutput(outputSockets[0], "_surfacematerial_", "ND_surfacematerial", "surfaceshader", "out", context);
    }

    ShaderPtr shader = std::make_shared<Shader>(name, graph);

    // Create our stage.
    ShaderStagePtr stage = createStage(Stage::PIXEL, *shader);
    stage->createUniformBlock(OSL::UNIFORMS);
    stage->createInputBlock(OSL::INPUTS);
    stage->createOutputBlock(OSL::OUTPUTS);

    // Create shader variables for all nodes that need this.
    createVariables(graph, context, *shader);

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

// TODO - determine it's better if this lives in ShaderGenerator as a useful API function
void OslShaderGenerator::emitAllDependentFunctionCalls(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    // Check if it's emitted already.
    if (!stage.isEmitted(node, context))
    {
        // Emit function calls for upstream connected nodes
        for (const auto& input : node.getInputs())
        {
            if (const auto& upstream = input->getConnectedSibling())
            {
                emitAllDependentFunctionCalls(*upstream, context, stage);
            }
        }
        stage.addFunctionCall(node, context);
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

void OslShaderGenerator::emitShaderInputs(const VariableBlock& inputs, ShaderStage& stage) const
{
    static const std::unordered_map<string, string> GEOMPROP_DEFINITIONS =
    {
        { "Pobject", "transform(\"object\", P)" },
        { "Pworld", "P" },
        { "Nobject", "transform(\"object\", N)" },
        { "Nworld", "N" },
        { "Tobject", "transform(\"object\", dPdu)" },
        { "Tworld", "dPdu" },
        { "Bobject", "transform(\"object\", dPdv)" },
        { "Bworld", "dPdv" },
        { "UV0", "{u,v}" },
        { "Vworld", "I" }
    };

    for (size_t i = 0; i < inputs.size(); ++i)
    {
        const ShaderPort* input = inputs[i];
        const string& type = _syntax->getTypeName(input->getType());

        if (input->getType() == Type::FILENAME)
        {
            // Shader inputs of type 'filename' (textures) need special handling.
            // In OSL codegen a 'filename' is translated to the custom type 'textureresource',
            // which is a struct containing a file string and a colorspace string.
            // For the published shader interface we here split this into two separate inputs,
            // which gives a nicer shader interface with widget metadata on each input.

            ValuePtr value = input->getValue();
            const string valueStr = value ? value->getValueString() : EMPTY_STRING;

            // Add the file string input
            emitLineBegin(stage);
            emitString("string " + input->getVariable() + " = \"" + valueStr + "\"", stage);
            emitMetadata(input, stage);
            emitString(",", stage);
            emitLineEnd(stage, false);

            // Add the colorspace string input
            emitLineBegin(stage);
            emitString("string " + input->getVariable() + "_colorspace = \"" + input->getColorSpace() + "\"", stage);
            emitLineEnd(stage, false);
            emitScopeBegin(stage, Syntax::DOUBLE_SQUARE_BRACKETS);
            emitLine("string widget = \"colorspace\"", stage, false);
            emitScopeEnd(stage, false, false);
        }
        else
        {
            emitLineBegin(stage);
            emitString(type + " " + input->getVariable(), stage);

            string value = _syntax->getValue(input, true);
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
            emitMetadata(input, stage);
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
        const TypeDesc outputType = output->getType();
        const string type = _syntax->getOutputTypeName(outputType);
        const string value = _syntax->getDefaultValue(outputType, true);
        const string& delim = (i == outputs.size() - 1) ? EMPTY_STRING : Syntax::COMMA;
        emitLine(type + " " + output->getVariable() + " = " + value + delim, stage, false);
    }
}

void OslShaderGenerator::emitMetadata(const ShaderPort* port, ShaderStage& stage) const
{
    static const std::unordered_map<TypeDesc, ShaderMetadata, TypeDesc::Hasher> UI_WIDGET_METADATA =
    {
        { Type::FLOAT, ShaderMetadata("widget", Type::STRING, Type::STRING.createValueFromStrings("number")) },
        { Type::INTEGER, ShaderMetadata("widget", Type::STRING, Type::STRING.createValueFromStrings("number")) },
        { Type::FILENAME, ShaderMetadata("widget", Type::STRING, Type::STRING.createValueFromStrings("filename")) },
        { Type::BOOLEAN, ShaderMetadata("widget", Type::STRING, Type::STRING.createValueFromStrings("checkBox")) }
    };

    static const std::set<TypeDesc> METADATA_TYPE_BLACKLIST =
    {
        Type::VECTOR2,  // Custom struct types doesn't support metadata declarations.
        Type::VECTOR4,  //
        Type::COLOR4,   //
        Type::FILENAME, //
        Type::BSDF      //
    };

    auto widgetMetadataIt = UI_WIDGET_METADATA.find(port->getType());
    const ShaderMetadata* widgetMetadata = widgetMetadataIt != UI_WIDGET_METADATA.end() ? &widgetMetadataIt->second : nullptr;
    const ShaderMetadataVecPtr& metadata = port->getMetadata();
    const string& geomprop = port->getGeomProp();

    if (widgetMetadata || (metadata && metadata->size()) || !geomprop.empty())
    {
        StringVec metadataLines;
        if (metadata)
        {
            for (size_t j = 0; j < metadata->size(); ++j)
            {
                const ShaderMetadata& data = (*metadata)[j];
                if (METADATA_TYPE_BLACKLIST.count(data.type) == 0)
                {
                    const string& delim = (widgetMetadata || j < metadata->size() - 1) ? Syntax::COMMA : EMPTY_STRING;
                    const string& dataType = _syntax->getTypeName(data.type);
                    const string dataValue = _syntax->getValue(data.type, *data.value, true);
                    metadataLines.emplace_back(dataType + " " + data.name + " = " + dataValue + delim);
                }
            }
        }
        if (widgetMetadata)
        {
            const string& delim = geomprop.empty() ? EMPTY_STRING : Syntax::COMMA;
            const string& dataType = _syntax->getTypeName(widgetMetadata->type);
            const string dataValue = _syntax->getValue(widgetMetadata->type, *widgetMetadata->value, true);
            metadataLines.emplace_back(dataType + " " + widgetMetadata->name + " = " + dataValue + delim);
        }
        if (!geomprop.empty())
        {
            const string& dataType = _syntax->getTypeName(Type::STRING);
            metadataLines.emplace_back(dataType + " mtlx_defaultgeomprop = \"" + geomprop + "\"");
        }
        if (metadataLines.size())
        {
            emitLineEnd(stage, false);
            emitScopeBegin(stage, Syntax::DOUBLE_SQUARE_BRACKETS);
            for (const auto& line : metadataLines)
            {
                emitLine(line, stage, false);
            }
            emitScopeEnd(stage, false, false);
        }
    }
}


void OslShaderGenerator::addSetCiTerminalNode(ShaderGraph& graph, ConstDocumentPtr document, GenContext& context) const
{
    string setCiNodeDefName = "ND_osl_set_ci";
    NodeDefPtr setCiNodeDef = document->getNodeDef(setCiNodeDefName);

    std::unordered_map<TypeDesc, ValuePtr, TypeDesc::Hasher> outputModeMap;
    int index = 0;
    for (auto input : setCiNodeDef->getInputs())
    {
        string inputName = input->getName();
        if (stringStartsWith(inputName, "input_"))
        {
            TypeDesc inputType = _typeSystem->getType(input->getType());
            outputModeMap[inputType] = std::make_shared<TypedValue<int>>(index++);
        }
    }

    for (auto output : graph.getOutputSockets())
    {
        auto outputType = output->getType();
        string typeName = outputType.getName();
        auto setCiNode = graph.inlineNodeBeforeOutput(output, "oslSetCi", setCiNodeDefName, "input_" + typeName, "out_ci", context);
        auto typeInput = setCiNode->getInput("output_mode");

        auto outputModeValue = outputModeMap[outputType];

        typeInput->setValue(outputModeValue);
    }
}


namespace OSL
{

// Identifiers for OSL variable blocks
const string UNIFORMS = "u";
const string INPUTS = "i";
const string OUTPUTS = "o";

} // namespace OSL

MATERIALX_NAMESPACE_END
