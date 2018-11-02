#include <MaterialXGenShader/Nodes/SourceCodeNodeImpl.h>
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

ShaderNodeImplPtr SourceCodeNodeImpl::create()
{
    return std::make_shared<SourceCodeNodeImpl>();
}

void SourceCodeNodeImpl::initialize(ElementPtr implementation, ShaderGenerator& shadergen)
{
    ShaderNodeImpl::initialize(implementation, shadergen);

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

void SourceCodeNodeImpl::emitFunctionDefinition(const ShaderNode& /*node*/, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Emit function definition for non-inlined functions
    if (!_inlined)
    {
        shader.addBlock(_functionSource, shadergen);
        shader.newLine();
    }

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

void SourceCodeNodeImpl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    if (_inlined)
    {
        // An inline function call

        static const string prefix("{{");
        static const string postfix("}}");

        // Inline expressions can only have a single output
        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        shader.addStr(" = ");

        size_t pos = 0;
        size_t i = _functionSource.find_first_of(prefix);
        while (i != string::npos)
        {
            shader.addStr(_functionSource.substr(pos, i - pos));

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
            shadergen.emitInput(context, input, shader);

            pos = j + 2;
            i = _functionSource.find_first_of(prefix, pos);
        }
        shader.addStr(_functionSource.substr(pos));
        shader.endLine();
    }
    else
    {
        // An ordinary source code function call

        // Declare the output variables
        for (size_t i = 0; i < node.numOutputs(); ++i)
        {
            shader.beginLine();
            shadergen.emitOutput(context, node.getOutput(i), true, true, shader);
            shader.endLine();
        }

        shader.beginLine();

        // Emit function name
        shader.addStr(_functionName + context.getFunctionSuffix() + "(");

        // Emit function inputs
        string delim = "";

        // Add any extra argument inputs first...
        for (const Argument& arg : context.getArguments())
        {
            shader.addStr(delim + arg.second);
            delim = ", ";
        }

        // ...and then all inputs on the node
        for (ShaderInput* input : node.getInputs())
        {
            shader.addStr(delim);
            shadergen.emitInput(context, input, shader);
            delim = ", ";
        }

        // Emit function outputs
        for (size_t i = 0; i < node.numOutputs(); ++i)
        {
            shader.addStr(delim);
            shadergen.emitOutput(context, node.getOutput(i), false, false, shader);
            delim = ", ";
        }

        // End function call
        shader.addStr(")");
        shader.endLine();
    }

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
