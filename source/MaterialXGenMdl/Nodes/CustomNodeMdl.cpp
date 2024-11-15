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

void CustomCodeNodeMdl::initialize(const InterfaceElement& element, GenContext& context)
{
    SourceCodeNodeMdl::initialize(element, context);

    // format the function source in a way that the ShaderCodeNodeMdl (the base class of the current one) can deal with it
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    const MdlShaderGenerator& shadergenMdl = static_cast<const MdlShaderGenerator&>(shadergen);
    const MdlSyntax& syntax = static_cast<const MdlSyntax&>(shadergen.getSyntax());
    const string uniformPrefix = syntax.getUniformQualifier() + " ";

    const Implementation& impl = static_cast<const Implementation&>(element);
    string moduleName = impl.getAttribute("file");
    if (moduleName.empty())
    {
        throw ExceptionShaderGenError("No source file was specified for the implementation '" + impl.getName() + "'");
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
        [&mdlModuleName, versionSuffix, &syntax](const string& marker)
    {
        return marker == syntax.getMdlVersionSuffixMarker() ? versionSuffix : marker;
    });

    // construct the fully qualified function name
    _functionSource = _qualifiedModuleName.substr(2) + "::" + _functionName + "(";

    // function parameters
    const StringSet& reservedWords = syntax.getReservedWords();
    const NodeDefPtr node = impl.getNodeDef();
    string delim = EMPTY_STRING;
    for (const InputPtr input : node->getInputs())
    {
        string inputName = input->getName();
        if (reservedWords.find(inputName) != reservedWords.end())
        {
            // add an "mxp_" prefix in case the field name is a reserved word
            inputName = "mxp_" + inputName;
        }
        _functionSource += delim + inputName + ": {{" + input->getName() + "}}";
        if (delim == EMPTY_STRING)
            delim = Syntax::COMMA + " ";
    }
    _functionSource += ")";
    _inlined = true;
}

MATERIALX_NAMESPACE_END