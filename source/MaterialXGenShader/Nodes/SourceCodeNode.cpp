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

void SourceCodeNode::emitFunctionDefinition(const ShaderNode& /*node*/, ShaderGenerator& shadergen, Shader& shader)
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

void SourceCodeNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

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

            if (input->connection)
            {
                string inputStr;
                shadergen.getInput(context, input, inputStr);
                code.push_back(inputStr);
            }
            else
            {
                string variableName = node.getName() + "_" + input->name + "_tmp";
                if (!variableNames.count(variableName))
                {
                    Shader::Variable newVariable(input->type, variableName, EMPTY_STRING, EMPTY_STRING, input->value);
                    shader.beginLine();
                    shadergen.emitVariable(newVariable, shadergen.getSyntax()->getConstantQualifier(), shader);
                    shader.endLine();
                    variableNames.insert(variableName);
                }
                code.push_back(variableName);
            }

            pos = j + 2;
            i = _functionSource.find_first_of(prefix, pos);
        }
        code.push_back(_functionSource.substr(pos));
        code.push_back(ShaderGenerator::SEMICOLON_NEWLINE);

        shader.beginLine();
        shadergen.emitOutput(context, node.getOutput(), true, false, shader);
        shader.addStr(" = ");
        for (const string& c : code)
        {
            shader.addStr(c);
        }
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
