//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMdl/Nodes/CustomNodeMdl.h>

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

void CustomCodeNodeMdl::initialize(const InterfaceElement& element, GenContext& context)
{
    // TODO this is missing struct return support!
    // SourceCodeNode::initialize without fetching the source code from file
    // this is part of the MDL compiler work later
    {
        ShaderNodeImpl::initialize(element, context);

        if (!element.isA<Implementation>())
        {
            throw ExceptionShaderGenError("Element '" + element.getName() + "' is not an Implementation element");
        }

        const Implementation& impl = static_cast<const Implementation&>(element);

        // Find the function name to use
        // If no function is given the source will be inlined.
        _functionName = impl.getAttribute("function");
        _inlined = _functionName.empty();
        if (!_inlined)
        {
            // Make sure the function name is valid.
            string validFunctionName = _functionName;
            context.getShaderGenerator().getSyntax().makeValidName(validFunctionName);
            if (_functionName != validFunctionName)
            {
                throw ExceptionShaderGenError("Function name '" + _functionName +
                                              "' used by implementation '" + impl.getName() + "' is not a valid identifier.");
            }
        }
        else
        {
            _functionSource = replaceSubstrings(_functionSource, { { "\n", "" } });
        }

        // Set hash using the function name.
        // TODO: Could be improved to include the full function signature.
        _hash = std::hash<string>{}(_functionName);
    }

    // format the function source in a way that the ShaderCodeNodeMdl (the base class of the current one) can deal with it
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    const Syntax& syntax = shadergen.getSyntax();
    const string uniformPrefix = syntax.getUniformQualifier() + " ";

    const Implementation& impl = static_cast<const Implementation&>(element);
    string moduleName = impl.getAttribute("function");
    string mdlModuleName = replaceSubstrings(moduleName, { { "/", "::" } });
    if (!stringStartsWith(mdlModuleName, "::"))
    {
        mdlModuleName = "::" + mdlModuleName;
    }

    if (!stringEndsWith(mdlModuleName, ".mdl"))
    {
        // error
    }
    else
    {
        mdlModuleName = mdlModuleName.substr(mdlModuleName.size() - 4);
    }
    _qualifiedModuleName = mdlModuleName;

    // construct the fully qualified function name
    _functionSource = _qualifiedModuleName.substr(2) + "::" + _functionName + "(";

    // function parameters
    const NodeDefPtr node = impl.getNodeDef();
    string delim = EMPTY_STRING;
    for (const InputPtr input : node->getInputs())
    {
        const string& inputName = input->getName();
        _functionSource += delim + input->getName() + ": {{" + inputName + "}}";
        if (delim == EMPTY_STRING)
            delim = Syntax::COMMA + " ";
    }
    _functionSource += ")";
    _inlined = true;
}

MATERIALX_NAMESPACE_END