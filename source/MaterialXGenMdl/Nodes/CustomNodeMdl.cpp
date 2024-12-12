//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMdl/Nodes/CustomNodeMdl.h>
#include <MaterialXGenMdl/MdlSyntax.h>
#include <MaterialXGenMdl/MdlShaderGenerator.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Util.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr CustomCodeNodeMdl::create()
{
    return std::make_shared<CustomCodeNodeMdl>();
}

const string& CustomCodeNodeMdl::getQualifiedModuleName() const
{
    return _qualifiedModuleName;
}

string CustomCodeNodeMdl::modifyPortName(const string& name, const MdlSyntax& syntax) const
{
    if (_useExternalSourceCode)
    {
        const StringSet& reservedWords = syntax.getReservedWords();
        if (reservedWords.find(name) == reservedWords.end())
        {
            // Use existing MDL parameter names if they don't collide with a reserved word.
            // This allows us to reference MDL existing functions without changing the MDL source code.
            return name;
        }
    }
    return syntax.modifyPortName(name);
}

void CustomCodeNodeMdl::initialize(const InterfaceElement& element, GenContext& context)
{
    SourceCodeNodeMdl::initialize(element, context);
    if (_inlined)
    {
        _useExternalSourceCode = false;
        initializeForInlineSourceCode(element, context);
    }
    else
    {
        _useExternalSourceCode = true;
        initializeForExternalSourceCode(element, context);
    }
}

void CustomCodeNodeMdl::initializeForInlineSourceCode(const InterfaceElement& element, GenContext& context)
{
    const Implementation& impl = static_cast<const Implementation&>(element);
    // Store the inline source because the `_functionSource` is used for the function call template string
    // that matched the regular MaterialX to MDL function mapping.
    _inlineSourceCode = impl.getAttribute("sourcecode");
    if (_inlineSourceCode.empty())
    {
        throw ExceptionShaderGenError("No source code was specified for the implementation '" + impl.getName() + "'");
    }
    if (_inlineSourceCode.find("//") != string::npos)
    {
        throw ExceptionShaderGenError("Source code contains unsupported comments '//', please use '/* comment */' instead in '" + impl.getName() + "'");
    }

    NodeDefPtr nodeDef = impl.getNodeDef();
    _inlineFunctionName = nodeDef->getName();
    _hash = std::hash<string>{}(_inlineFunctionName); // make sure we emit the function definition only once

    const ShaderGenerator& shadergen = context.getShaderGenerator();
    const MdlSyntax& syntax = static_cast<const MdlSyntax&>(shadergen.getSyntax());
    // Construct the function call template string
    initializeFunctionCallTemplateString(syntax, *nodeDef);
    // Collect information about output names and defaults
    initializeOutputDefaults(syntax, *nodeDef);
}

void CustomCodeNodeMdl::initializeForExternalSourceCode(const InterfaceElement& element, GenContext& context)
{
    // Format the function source in a way that the ShaderCodeNodeMdl (the base class of the current one) can deal with it
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    const MdlShaderGenerator& shadergenMdl = static_cast<const MdlShaderGenerator&>(shadergen);
    const MdlSyntax& syntax = static_cast<const MdlSyntax&>(shadergen.getSyntax());
    const string uniformPrefix = syntax.getUniformQualifier() + " ";

    // Map `file` to a qualified MDL module name
    const Implementation& impl = static_cast<const Implementation&>(element);
    string moduleName = impl.getAttribute("file");
    if (moduleName.empty())
    {
        throw ExceptionShaderGenError("No source file was specified for the implementation '" + impl.getName() + "'");
    }
    if (_functionName.empty())
    {
        throw ExceptionShaderGenError("No function name was specified for the implementation '" + impl.getName() + "'");
    }

    string mdlModuleName = replaceSubstrings(moduleName, { { "/", "::" } });
    if (!stringStartsWith(mdlModuleName, "::"))
    {
        mdlModuleName = "::" + mdlModuleName;
    }

    if (!stringEndsWith(mdlModuleName, ".mdl"))
    {
        throw ExceptionShaderGenError("Referenced source file is not an MDL module: '" + moduleName +
                                      "' used by implementation '" + impl.getName() + "'");
    }
    else
    {
        mdlModuleName = mdlModuleName.substr(0, mdlModuleName.size() - 4);
    }
    const string versionSuffix = shadergenMdl.getMdlVersionFilenameSuffix(context);
    _qualifiedModuleName = syntax.replaceSourceCodeMarkers(element.getName(), mdlModuleName,
        [&versionSuffix, &syntax](const string& marker)
    {
        return marker == syntax.getMdlVersionSuffixMarker() ? versionSuffix : marker;
    });

    NodeDefPtr nodeDef = impl.getNodeDef();
    // Construct the function call template string
    initializeFunctionCallTemplateString(syntax, *nodeDef);
    // Collect information about output names and defaults
    initializeOutputDefaults(syntax, *nodeDef);
}

void CustomCodeNodeMdl::initializeFunctionCallTemplateString(const MdlSyntax& syntax, const NodeDef& nodeDef)
{
    // Construct the fully qualified function name for external functions
    if (_useExternalSourceCode)
    {
        _functionSource = _qualifiedModuleName.substr(2) + "::" + _functionName + "(";
    }
    // or simple name for local functions
    else
    {
        _functionSource = _inlineFunctionName + "(";
    }

    // Function parameters
    string delim = EMPTY_STRING;
    for (const InputPtr& input : nodeDef.getInputs())
    {
        string inputName = modifyPortName(input->getName(), syntax);
        _functionSource += delim + inputName + ": {{" + input->getName() + "}}";
        if (delim == EMPTY_STRING)
            delim = Syntax::COMMA + " ";
    }
    _functionSource += ")";
    _inlined = true;
}

void CustomCodeNodeMdl::initializeOutputDefaults(const MdlSyntax&, const NodeDef& nodeDef)
{
    for (const OutputPtr& output : nodeDef.getOutputs())
    {
        _outputDefaults.push_back(output->getValue());
    }
}

void CustomCodeNodeMdl::emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    // No source code printing for externally defined functions
    if (_useExternalSourceCode)
    {
        return;
    }

    const ShaderGenerator& shadergen = context.getShaderGenerator();
    const MdlSyntax& syntax = static_cast<const MdlSyntax&>(shadergen.getSyntax());
    shadergen.emitComment("generated code for implementation: '" + node.getImplementation().getName() + "'", stage);

    // Function return type
    struct Field
    {
        string name;
        string type_name;
        string default_value;
    };
    vector<Field> outputs;
    size_t i = 0;
    for (const ShaderOutput* output : node.getOutputs())
    {
        string name = modifyPortName(output->getName(), syntax);
        TypeDesc type = output->getType();
        const ValuePtr defaultValue = _outputDefaults[i];
        outputs.push_back({
            name,
            syntax.getTypeName(type),
            defaultValue ? syntax.getValue(type, *defaultValue.get()) : syntax.getDefaultValue(type)
        });
        ++i;
    }

    size_t numOutputs = node.getOutputs().size();
    string returnTypeName;
    if (numOutputs == 1)
    {
        returnTypeName = outputs.back().type_name;
    }
    else
    {
        returnTypeName = _inlineFunctionName + "_return_type";
        shadergen.emitLine("struct " + returnTypeName, stage, false);
        shadergen.emitScopeBegin(stage, Syntax::CURLY_BRACKETS);
        for (const auto& field : outputs)
        {
            // ignore the default values here, they have to be initialized in the body
            shadergen.emitLine(field.type_name + " " + field.name, stage);
        }
        shadergen.emitScopeEnd(stage, Syntax::CURLY_BRACKETS);
        shadergen.emitLineEnd(stage, false);
    }
    // Signature
    shadergen.emitString(returnTypeName + " " + _inlineFunctionName, stage);
    {
        // Function parameters
        shadergen.emitScopeBegin(stage, Syntax::PARENTHESES);
        size_t paramCount = node.getInputs().size();
        const string uniformPrefix = syntax.getUniformQualifier() + " ";
        for (const ShaderInput* input : node.getInputs())
        {
            const string& qualifier = input->isUniform() || input->getType() == Type::FILENAME ? uniformPrefix : EMPTY_STRING;
            const string& type = syntax.getTypeName(input->getType());
            const string name = modifyPortName(input->getName(), syntax);
            const string& delim = --paramCount == 0 ? EMPTY_STRING : Syntax::COMMA;
            shadergen.emitString("    " + qualifier + type + " " + name + delim + Syntax::NEWLINE, stage);
        }
        shadergen.emitScopeEnd(stage, false, true);
    }
    {
        // Function body
        shadergen.emitScopeBegin(stage, Syntax::CURLY_BRACKETS);

        // Out variable initialization
        shadergen.emitComment("initialize outputs:", stage);
        for (const auto& field : outputs)
        {
            shadergen.emitLine(field.type_name + " " + field.name + " = " + field.default_value, stage);
        }

        // User defined code
        shadergen.emitComment("inlined shader source code:", stage);
        shadergen.emitLine(_inlineSourceCode, stage, false);

        // Output packing
        shadergen.emitComment("pack (in case of multiple outputs) and return outputs:", stage);
        if (numOutputs == 1)
        {
            shadergen.emitLine("return " + outputs.back().name, stage, true);
        }
        else
        {
            // Return a constructor call of the return struct type
            shadergen.emitString("    return " + returnTypeName + "(", stage);
            string delim = EMPTY_STRING;
            for (const auto& field : outputs)
            {
                shadergen.emitString(delim + field.name, stage);
                if (delim == EMPTY_STRING)
                    delim = Syntax::COMMA + " ";
            }
            shadergen.emitString(")", stage);
            shadergen.emitLineEnd(stage, true);
        }
        shadergen.emitScopeEnd(stage, false, true);
    }
    shadergen.emitLine("", stage, false); // empty line for spacing
}

MATERIALX_NAMESPACE_END
