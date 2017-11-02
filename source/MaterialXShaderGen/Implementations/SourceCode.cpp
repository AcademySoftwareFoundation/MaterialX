#include <MaterialXCore/Library.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>

#include <MaterialXShaderGen/Implementations/SourceCode.h>
#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/Util.h>

#include <cstdarg>

namespace MaterialX
{

SgImplementationPtr SourceCode::creator()
{
    return std::make_shared<SourceCode>();
}

void SourceCode::initialize(const Implementation& implementation)
{
    SgImplementation::initialize(implementation);

    const string& file = implementation.getAttribute("file");
    if (file.empty())
    {
        throw ExceptionShaderGenError("No source file specified for implementation '" + implementation.getName() + "'");
    }

    _functionSource = "";
    _inlined = (getFileExtension(file) == "inline");

    // Find the function name to use
    _functionName = implementation.getAttribute("function");
    if (_functionName.empty())
    {
        // No function given so use nodedef name
        _functionName = implementation.getNodeDef();
    }

    if (!readFile(ShaderGenerator::findSourceCode(file), _functionSource))
    {
        throw ExceptionShaderGenError("Can't find source file '" + file + "' used by implementation '" + implementation.getName() + "'");
    }

    if (_inlined)
    {
        _functionSource.erase(std::remove(_functionSource.begin(), _functionSource.end(), '\n'), _functionSource.end());
    }

    string extra = implementation.getAttribute("sx_extra_inputs");
    if (!extra.empty())
    {
        extra.erase(std::remove(extra.begin(), extra.end(), ' '), extra.end());
        std::replace(extra.begin(), extra.end(), ',', ' ');
        std::stringstream ss(extra);
        std::istream_iterator<std::string> begin(ss);
        std::istream_iterator<std::string> end;
        _extraInputs.assign(begin, end);
    }
}

void SourceCode::emitFunction(const SgNode& /*node*/, ShaderGenerator& /*shadergen*/, Shader& shader)
{
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
                shader.addInclude(filename);
            }
            else
            {
                shader.addLine(line, false);
            }
        }

        shader.newLine();
    }
}

void SourceCode::emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader, int numArgs, ...)
{
    if (_inlined)
    {
        // An inline function call

        static const string prefix("{{");
        static const string postfix("}}");

        // Inline expressions can only have a single output
        shader.beginLine();
        shadergen.emitOutput(node.getNode(), true, shader);
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
            const ValueElement& port = node.getPort(variable);
            shadergen.emitInput(port, shader);

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
        shadergen.emitOutput(node.getNode(), true, shader);
        shader.endLine();

        shader.beginLine();

        // Emit function name
        shader.addStr(_functionName + "(");

        // Emit function inputs
        string delim = "";

        // Add any extra argument inputs first...
        va_list argsList;
        va_start(argsList, numArgs);
        for (int i = 0; i < numArgs; i++)
        {
            const char* arg = va_arg(argsList, const char*);
            shader.addStr(delim + arg);
            delim = ", ";
        }
        va_end(argsList);

        // ...and then all inputs according to node definition
        for (ValueElementPtr elem : node.getNodeDef().getChildrenOfType<ValueElement>())
        {
            const ValueElement& port = node.getPort(elem->getName());
            shader.addStr(delim);
            shadergen.emitInput(port, shader);
            delim = ", ";
        }

        // Emit function output
        shader.addStr(delim);
        shadergen.emitOutput(node.getNode(), false, shader);

        // End function call
        shader.addStr(")");
        shader.endLine();
    }
}

} // namespace MaterialX
