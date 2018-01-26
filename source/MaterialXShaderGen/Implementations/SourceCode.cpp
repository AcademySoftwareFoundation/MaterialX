#include <MaterialXCore/Library.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>

#include <MaterialXShaderGen/Implementations/SourceCode.h>
#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/HwShader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/Util.h>

#include <cstdarg>

namespace MaterialX
{

SgImplementationPtr SourceCode::creator()
{
    return std::make_shared<SourceCode>();
}

void SourceCode::initialize(ElementPtr implementation, ShaderGenerator& shadergen)
{
    SgImplementation::initialize(implementation, shadergen);

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

void SourceCode::emitFunctionDefinition(const SgNode& /*node*/, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Emit function definition for non-inlined functions
    if (!_inlined)
    {
        static const string kIncludePattern = "#include ";

        std::stringstream stream(_functionSource);
        for (string line; std::getline(stream, line); )
        {
            size_t pos = line.find(kIncludePattern);
            if (pos != string::npos)
            {
                const size_t start = pos + kIncludePattern.size() + 1;
                const size_t count = line.size() - start - 1;
                const string filename = line.substr(start, count);
                shader.addInclude(filename, shadergen);
            }
            else
            {
                shader.addLine(line, false);
            }
        }

        shader.newLine();
    }

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

void SourceCode::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    if (_inlined)
    {
        // An inline function call

        static const string prefix("{{");
        static const string postfix("}}");

        // Inline expressions can only have a single output
        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), true, shader);
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
            const SgInput* input = node.getInput(variable);
            shadergen.emitInput(input, shader);

            pos = j + 2;
            i = _functionSource.find_first_of(prefix, pos);
        }
        shader.addStr(_functionSource.substr(pos));
        shader.endLine();
    }
    else
    {
        // An ordinary source code function call
        // TODO: Support multiple outputs

        // Declare the output variable
        shader.beginLine();
        shadergen.emitOutput(node.getOutput(), true, shader);
        shader.endLine();

        shader.beginLine();

        // Emit function name
        shader.addStr(_functionName + "(");

        // Emit function inputs
        string delim = "";

        // Add any extra argument inputs first...
        const Arguments* args = shadergen.getExtraArguments(node);
        if (args)
        {
            for (size_t i = 0; i < args->size(); ++i)
            {
                const Argument& arg = (*args)[i];
                shader.addStr(delim + arg.second);
                delim = ", ";
            }
        }

        // ...and then all inputs on the node
        for (SgInput* input : node.getInputs())
        {
            shader.addStr(delim);
            shadergen.emitInput(input, shader);
            delim = ", ";
        }

        // Emit function output
        shader.addStr(delim);
        shadergen.emitOutput(node.getOutput(), false, shader);

        // End function call
        shader.addStr(")");
        shader.endLine();
    }

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

} // namespace MaterialX
