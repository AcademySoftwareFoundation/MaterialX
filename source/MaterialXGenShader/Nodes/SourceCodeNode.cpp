#include <MaterialXGenShader/Nodes/SourceCodeNode.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>

#include <cstdarg>

namespace MaterialX
{

ShaderNodeImplPtr SourceCodeNode::create()
{
    return std::make_shared<SourceCodeNode>();
}

void SourceCodeNode::initialize(ElementPtr implementation, GenContext& context)
{
    ShaderNodeImpl::initialize(implementation, context);

    ImplementationPtr impl = implementation->asA<Implementation>();
    if (!impl)
    {
        throw ExceptionShaderGenError("Element '" + implementation->getName() + "' is not a source code implementation");
    }

    const string& file = impl->getAttribute("file");
    if (file.empty())
    {
        throw ExceptionShaderGenError("No source file specified for implementation '" + impl->getName() + "'");
    }

    _functionSource = "";
    _inlined = (getFileExtension(file) == "inline");

    // Find the function name to use
    _functionName = impl->getAttribute("function");
    if (_functionName.empty())
    {
        // No function given so use nodedef name
        _functionName = impl->getNodeDefString();
    }

    if (!readFile(context.findSourceCode(file), _functionSource))
    {
        throw ExceptionShaderGenError("Can't find source file '" + file + "' used by implementation '" + impl->getName() + "'");
    }

    if (_inlined)
    {
        _functionSource.erase(std::remove(_functionSource.begin(), _functionSource.end(), '\n'), _functionSource.end());
    }
}

void SourceCodeNode::emitFunctionDefinition(const ShaderNode&, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, MAIN_STAGE)
        // Emit function definition for non-inlined functions
        if (!_inlined && !_functionSource.empty())
        {
            const ShaderGenerator& shadergen = context.getShaderGenerator();
            shadergen.emitBlock(_functionSource, context, stage);
            shadergen.emitLineBreak(stage);
        }
    END_SHADER_STAGE(stage, MAIN_STAGE)
}

void SourceCodeNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, MAIN_STAGE)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        if (_inlined)
        {
            // An inline function call

            static const string prefix("{{");
            static const string postfix("}}");

            size_t pos = 0;
            size_t i = _functionSource.find_first_of(prefix);
            std::set<string> variableNames;
            vector<string> code;
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

                if (input->getConnection())
                {
                    code.push_back(shadergen.getUpstreamResult(input, context));
                }
                else
                {
                    string variableName = node.getName() + "_" + input->getName() + "_tmp";
                    if (!variableNames.count(variableName))
                    {
                        ShaderPort v(nullptr, input->getType(), variableName, input->getValue());
                        shadergen.emitLineBegin(stage);
                        const Syntax& syntax = shadergen.getSyntax();
                        const string valueStr = (v.getValue() ? syntax.getValue(v.getType(), *v.getValue()) : syntax.getDefaultValue(v.getType()));
                        string str = syntax.getConstantQualifier() + " " + syntax.getTypeName(v.getType()) + " " + v.getVariable();
                        str += valueStr.empty() ? EMPTY_STRING : " = " + valueStr;
                        shadergen.emitString(str, stage);
                        shadergen.emitLineEnd(stage);
                        variableNames.insert(variableName);
                    }
                    code.push_back(variableName);
                }

                pos = j + 2;
                i = _functionSource.find_first_of(prefix, pos);
            }
            code.push_back(_functionSource.substr(pos));

            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(node.getOutput(), true, false, context, stage);
            shadergen.emitString(" = ", stage);
            for (const string& c : code)
            {
                shadergen.emitString(c, stage);
            }
            shadergen.emitLineEnd(stage);
        }
        else
        {
            // An ordinary source code function call

            // Declare the output variables
            for (size_t i = 0; i < node.numOutputs(); ++i)
            {
                shadergen.emitLineBegin(stage);
                shadergen.emitOutput(node.getOutput(i), true, true, context, stage);
                shadergen.emitLineEnd(stage);
            }

            shadergen.emitLineBegin(stage);
            string delim = "";

            // Emit function name.
            shadergen.emitString(_functionName + "(", stage);

            // Emit all inputs on the node.
            for (ShaderInput* input : node.getInputs())
            {
                shadergen.emitString(delim, stage);
                shadergen.emitInput(input, context, stage);
                delim = ", ";
            }

            // Emit node outputs.
            for (size_t i = 0; i < node.numOutputs(); ++i)
            {
                shadergen.emitString(delim, stage);
                shadergen.emitOutput(node.getOutput(i), false, false, context, stage);
                delim = ", ";
            }

            // End function call
            shadergen.emitString(")", stage);
            shadergen.emitLineEnd(stage);
        }
    END_SHADER_STAGE(stage, MAIN_STAGE)
}

} // namespace MaterialX
