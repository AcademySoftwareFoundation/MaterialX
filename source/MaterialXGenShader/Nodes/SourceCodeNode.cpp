#include <MaterialXGenShader/Nodes/SourceCodeNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/ShaderGenerator.h>
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

void SourceCodeNode::initialize(ElementPtr implementation, ShaderGenerator& shadergen, const GenOptions& options)
{
    ShaderNodeImpl::initialize(implementation, shadergen, options);

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

    if (!readFile(shadergen.findSourceCode(file), _functionSource))
    {
        throw ExceptionShaderGenError("Can't find source file '" + file + "' used by implementation '" + impl->getName() + "'");
    }

    if (_inlined)
    {
        _functionSource.erase(std::remove(_functionSource.begin(), _functionSource.end(), '\n'), _functionSource.end());
    }
}

void SourceCodeNode::emitFunctionDefinition(ShaderStage& stage, const ShaderNode&, ShaderGenerator& shadergen)
{
BEGIN_SHADER_STAGE(stage, MAIN_STAGE)

    // Emit function definition for non-inlined functions
    if (!_inlined)
    {
        shadergen.emitBlock(stage, _functionSource);
        shadergen.emitLineEnd(stage);
    }

END_SHADER_STAGE(stage, MAIN_STAGE)
}

void SourceCodeNode::emitFunctionCall(ShaderStage& stage, const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen)
{
BEGIN_SHADER_STAGE(stage, MAIN_STAGE)
    if (_inlined)
    {
        // An inline function call

        static const string prefix("{{");
        static const string postfix("}}");

        // Inline expressions can only have a single output
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(stage, context, node.getOutput(), true, false);
        shadergen.emitString(stage, " = ");

        size_t pos = 0;
        size_t i = _functionSource.find_first_of(prefix);
        while (i != string::npos)
        {
            shadergen.emitString(stage, _functionSource.substr(pos, i - pos));

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
            shadergen.emitInput(stage, context, input);

            pos = j + 2;
            i = _functionSource.find_first_of(prefix, pos);
        }
        shadergen.emitString(stage, _functionSource.substr(pos));
        shadergen.emitLineEnd(stage);
    }
    else
    {
        // An ordinary source code function call

        // Declare the output variables
        for (size_t i = 0; i < node.numOutputs(); ++i)
        {
            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(stage, context, node.getOutput(i), true, true);
            shadergen.emitLineEnd(stage);
        }

        shadergen.emitLineBegin(stage);

        // Emit function name
        shadergen.emitString(stage, _functionName + context.getFunctionSuffix() + "(");

        // Emit function inputs
        string delim = "";

        // Add any extra argument inputs first...
        for (const Argument& arg : context.getArguments())
        {
            shadergen.emitString(stage, delim + arg.second);
            delim = ", ";
        }

        // ...and then all inputs on the node
        for (ShaderInput* input : node.getInputs())
        {
            shadergen.emitString(stage, delim);
            shadergen.emitInput(stage, context, input);
            delim = ", ";
        }

        // Emit function outputs
        for (size_t i = 0; i < node.numOutputs(); ++i)
        {
            shadergen.emitString(stage, delim);
            shadergen.emitOutput(stage, context, node.getOutput(i), false, false);
            delim = ", ";
        }

        // End function call
        shadergen.emitString(stage, ")");
        shadergen.emitLineEnd(stage);
    }
END_SHADER_STAGE(stage, MAIN_STAGE)
}

} // namespace MaterialX
