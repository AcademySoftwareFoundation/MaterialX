//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMdl/Nodes/SourceCodeNodeMdl.h>
#include <MaterialXGenMdl/MdlSyntax.h>
#include <MaterialXGenMdl/MdlShaderGenerator.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXFormat/Util.h>

#include <numeric>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr SourceCodeNodeMdl::create()
{
    return std::make_shared<SourceCodeNodeMdl>();
}

void SourceCodeNodeMdl::resolveSourceCode(const InterfaceElement& /*element*/, GenContext& /*context*/)
{
    // Initialize without fetching the source code from file.
    // The resolution of MDL modules is done by the MDL compiler when loading the generated source code.
    // All references MDL modules must be accessible via MDL search paths set up by the consuming application.
}

void SourceCodeNodeMdl::initialize(const InterfaceElement& element, GenContext& context)
{
    SourceCodeNode::initialize(element, context);
    const MdlSyntax& syntax = static_cast<const MdlSyntax&>(context.getShaderGenerator().getSyntax());

    const Implementation& impl = static_cast<const Implementation&>(element);
    NodeDefPtr nodeDef = impl.getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Can't find nodedef for implementation element " + element.getName());
    }

    _returnStruct = EMPTY_STRING;
    if (nodeDef->getOutputCount() > 1)
    {
        if (_functionName.empty())
        {
            size_t pos = _functionSource.find_first_of('(');
            string functionName = _functionSource.substr(0, pos);

            const ShaderGenerator& shadergen = context.getShaderGenerator();
            const MdlShaderGenerator& shadergenMdl = static_cast<const MdlShaderGenerator&>(shadergen);
            const string versionSuffix = shadergenMdl.getMdlVersionFilenameSuffix(context);
            functionName = syntax.replaceSourceCodeMarkers(getName(), functionName, [&versionSuffix, syntax](const string& marker)
            {
                return marker == syntax.getMdlVersionSuffixMarker() ? versionSuffix : EMPTY_STRING;
            });
            _returnStruct = functionName + "__result";
        }
        else
        {
            _returnStruct = _functionName + "__result";
        }
    }
}

void SourceCodeNodeMdl::emitFunctionDefinition(const ShaderNode&, GenContext&, ShaderStage&) const
{
}

void SourceCodeNodeMdl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const MdlShaderGenerator& shadergenMdl = static_cast<const MdlShaderGenerator&>(shadergen);
        if (_inlined)
        {
            const MdlSyntax& syntax = static_cast<const MdlSyntax&>(shadergenMdl.getSyntax());
            const string versionSuffix = shadergenMdl.getMdlVersionFilenameSuffix(context);
            string code = syntax.replaceSourceCodeMarkers(node.getName(), _functionSource,
                                                             [&shadergenMdl, &context, &node, &versionSuffix, syntax](const string& marker)
            {
                // Special handling for the version suffix of MDL source code modules.
                if (marker == syntax.getMdlVersionSuffixMarker())
                {
                    return versionSuffix;
                }
                // Insert inputs based on parameter names.
                else
                {
                    const ShaderInput* input = node.getInput(marker);
                    if (!input)
                    {
                        throw ExceptionShaderGenError("Could not find an input named '" + marker +
                                                      "' on node '" + node.getName() + "'");
                    }

                    return shadergenMdl.getUpstreamResult(input, context);
                }
            });

            if (!_returnStruct.empty())
            {
                // Emit the struct multioutput.
                const string resultVariableName = node.getName() + "_result";
                shadergen.emitLineBegin(stage);
                shadergen.emitString("auto " + resultVariableName + " = ", stage);
            }
            else
            {
                // Emit the single output.
                shadergen.emitLineBegin(stage);
                shadergen.emitOutput(node.getOutput(0), true, false, context, stage);
                shadergen.emitString(" = ", stage);
            }

            shadergen.emitString(code, stage);
            shadergen.emitLineEnd(stage);
        }
        else
        {
            // An ordinary source code function call

            if (!_returnStruct.empty())
            {
                // Emit the struct multioutput.
                const string resultVariableName = node.getName() + "_result";
                shadergen.emitLineBegin(stage);
                shadergen.emitString("auto " + resultVariableName + " = ", stage);
            }
            else
            {
                // Emit the single output.
                shadergen.emitLineBegin(stage);
                shadergen.emitOutput(node.getOutput(0), true, false, context, stage);
                shadergen.emitString(" = ", stage);
            }

            // Emit function name.
            shadergen.emitString(_functionName + "(", stage);

            // Emit all inputs on the node.
            string delim = "";
            for (ShaderInput* input : node.getInputs())
            {
                shadergen.emitString(delim, stage);
                shadergen.emitInput(input, context, stage);
                delim = ", ";
            }

            // End function call
            shadergen.emitString(")", stage);
            shadergen.emitLineEnd(stage);
        }
    }
}

MATERIALX_NAMESPACE_END
