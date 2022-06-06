//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenMdl/Nodes/SourceCodeNodeMdl.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Util.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr SourceCodeNodeMdl::create()
{
    return std::make_shared<SourceCodeNodeMdl>();
}

void SourceCodeNodeMdl::initialize(const InterfaceElement& element, GenContext& context)
{
    SourceCodeNode::initialize(element, context);

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
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        if (_inlined)
        {
            // An inline function call

            static const string prefix("{{");
            static const string postfix("}}");

            size_t pos = 0;
            size_t i = _functionSource.find_first_of(prefix);
            StringVec code;
            while (i != string::npos)
            {
                code.push_back(_functionSource.substr(pos, i - pos));
                size_t j = _functionSource.find_first_of(postfix, i + 2);
                if (j == string::npos)
                {
                    throw ExceptionShaderGenError("Malformed inline expression in implementation for node " + node.getName());
                }

                const string variable = _functionSource.substr(i + 2, j - i - 2);
                const ShaderInput* input = node.getInput(variable);
                if (!input)
                {
                    throw ExceptionShaderGenError("Could not find an input named '" + variable +
                        "' on node '" + node.getName() + "'");
                }

                code.push_back(shadergen.getUpstreamResult(input, context));

                pos = j + 2;
                i = _functionSource.find_first_of(prefix, pos);
            }
            code.push_back(_functionSource.substr(pos));

            if (!_returnStruct.empty())
            {
                // Emit the struct multioutput.
                const string resultVariableName = node.getName() + "_result";
                shadergen.emitLineBegin(stage);
                shadergen.emitString(_returnStruct + " " + resultVariableName + " = ", stage);
            }
            else
            {
                // Emit the single output.
                shadergen.emitLineBegin(stage);
                shadergen.emitOutput(node.getOutput(0), true, false, context, stage);
                shadergen.emitString(" = ", stage);
            }

            for (const string& c : code)
            {
                shadergen.emitString(c, stage);
            }
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
                shadergen.emitString(_returnStruct + " " + resultVariableName + " = ", stage);
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
    END_SHADER_STAGE(stage, Stage::PIXEL)
}

MATERIALX_NAMESPACE_END
