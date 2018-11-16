#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Syntax.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/Nodes/CompareNode.h>
#include <MaterialXGenShader/Nodes/SwitchNode.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Value.h>

namespace MaterialX
{

const string Shader::PRIVATE_UNIFORMS = "PrivateUniforms";
const string Shader::PUBLIC_UNIFORMS = "PublicUniforms";

Shader::Shader(const string& name)
    : _name(name)
    , _rootGraph(nullptr)
    , _activeStage(PIXEL_STAGE)
    , _appData("AppData", "ad")
{
    _stages.push_back(Stage("Pixel"));

    // Create default uniform blocks for pixel stage
    createUniformBlock(PIXEL_STAGE, PRIVATE_UNIFORMS, "prvUniform");
    createUniformBlock(PIXEL_STAGE, PUBLIC_UNIFORMS, "pubUniform");
}

void Shader::initialize(ElementPtr element, ShaderGenerator& shadergen, const GenOptions& options)
{
    // Create our shader generation root graph
    _rootGraph = ShaderGraph::create(_name, element, shadergen, options);

    // Make it active
    pushActiveGraph(_rootGraph.get());

    // Create shader variables for all nodes that need this (geometric nodes / input streams)
    for (ShaderNode* node : _rootGraph->getNodes())
    {
        ShaderNodeImpl* impl = node->getImplementation();
        impl->createVariables(*node, shadergen, *this);
    }

    // Create uniforms for the published graph interface
    for (ShaderGraphInputSocket* inputSocket : _rootGraph->getInputSockets())
    {
        // Only for inputs that are connected/used internally
        if (inputSocket->connections.size())
        {
            if (_rootGraph->isEditable(*inputSocket))
            {
                // Create the uniform
                createUniform(PIXEL_STAGE, PUBLIC_UNIFORMS, inputSocket->type, inputSocket->variable, EMPTY_STRING, inputSocket->value);
            }
        }
    }
}

void Shader::beginScope(Brackets brackets)
{
    Stage& s = stage();
        
    switch (brackets) {
    case Brackets::BRACES:
        indent();
        s.code += "{\n";
        break;
    case Brackets::PARENTHESES:
        indent();
        s.code += "(\n";
        break;
    case Brackets::SQUARES:
        indent();
        s.code += "[\n";
        break;
    case Brackets::NONE:
        break;
    }

    ++s.indentations;
    s.scopes.push(brackets);
}

void Shader::endScope(bool semicolon, bool newline)
{
    Stage& s = stage();

    if (s.scopes.empty())
    {
        throw ExceptionShaderGenError("End scope called with no scope active, please check your beginScope/endScope calls");
    }

    Brackets brackets = s.scopes.back();
    s.scopes.pop();
    --s.indentations;

    switch (brackets) {
    case Brackets::BRACES:
        indent();
        s.code += "}";
        break;
    case Brackets::PARENTHESES:
        indent();
        s.code += ")";
        break;
    case Brackets::SQUARES:
        indent();
        s.code += "]";
        break;
    case Brackets::NONE:
        break;
    }
    if (semicolon)
        s.code += ";";
    if (newline)
        s.code += "\n";
}

void Shader::beginLine()
{
    indent();
}

void Shader::endLine(bool semicolon)
{
    stage().code += semicolon ? ";\n" : "\n";
}

void Shader::newLine()
{
    stage().code += "\n";
}

void Shader::addStr(const string& str)
{
    stage().code += str;
}

void Shader::addLine(const string& str, bool semicolon)
{
    beginLine();
    stage().code += str;
    endLine(semicolon);
}

void Shader::addComment(const string& str)
{
    beginLine();
    stage().code += "// " + str;
    endLine(false);
}

void Shader::addBlock(const string& str, ShaderGenerator& shadergen)
{
    static const string INCLUDE_PATTERN = "#include ";

    // Add each line in the block seperatelly
    // to get correct indentation
    std::stringstream stream(str);
    for (string line; std::getline(stream, line); )
    {
        size_t pos = line.find(INCLUDE_PATTERN);
        if (pos != string::npos)
        {
            const size_t start = pos + INCLUDE_PATTERN.size() + 1;
            const size_t count = line.size() - start - 1;
            const string filename = line.substr(start, count);
            addInclude(filename, shadergen);
        }
        else
        {
            addLine(line, false);
        }
    }
}

void Shader::addFunctionDefinition(ShaderNode* node, ShaderGenerator& shadergen)
{
    Stage& s = stage();
    ShaderNodeImpl* impl = node->getImplementation();
    if (s.definedFunctions.find(impl) == s.definedFunctions.end())
    {
        s.definedFunctions.insert(impl);
        impl->emitFunctionDefinition(*node, shadergen, *this);
    }
}

void Shader::addFunctionCall(ShaderNode* node, const GenContext& context, ShaderGenerator& shadergen)
{
    ShaderNodeImpl* impl = node->getImplementation();
    impl->emitFunctionCall(*node, const_cast<GenContext&>(context), shadergen, *this);
}

void Shader::addInclude(const string& file, ShaderGenerator& shadergen)
{
    const string path = shadergen.findSourceCode(file);

    Stage& s = stage();
    if (s.includes.find(path) == s.includes.end())
    {
        string content;
        if (!readFile(path, content))
        {
            throw ExceptionShaderGenError("Could not find include file: '" + file + "'");
        }
        s.includes.insert(path);
        addBlock(content, shadergen);
    }
}

void Shader::indent()
{
    // Use 4 spaces as default indentation
    static const string INDENTATION = "    ";
    Stage& s = stage();
    for (int i = 0; i < s.indentations; ++i)
    {
        s.code += INDENTATION;
    }
}

void Shader::createConstant(size_t stage, const TypeDesc* type, const string& name, const string& semantic, ValuePtr value)
{
    Stage& s = _stages[stage];
    if (s.constants.variableMap.find(name) == s.constants.variableMap.end())
    {
        VariablePtr variablePtr = std::make_shared<Variable>(type, name, semantic, value);
        s.constants.variableMap[name] = variablePtr;
        s.constants.variableOrder.push_back(variablePtr.get());
    }
}

const Shader::VariableBlock& Shader::getConstantBlock(size_t stage) const
{
    const Stage& s = _stages[stage];
    return s.constants;
}

void Shader::createUniformBlock(size_t stage, const string& block, const string& instance)
{
    Stage& s = _stages[stage];
    auto it = s.uniforms.find(block);
    if (it == s.uniforms.end())
    {
        s.uniforms[block] = std::make_shared<VariableBlock>(block, instance);
    }
}

void Shader::createUniform(size_t stage, const string& block, const TypeDesc* type, const string& name, const string& semantic, ValuePtr value)
{
    const Stage& s = _stages[stage];
    auto it = s.uniforms.find(block);
    if (it == s.uniforms.end())
    {
        throw ExceptionShaderGenError("No uniform block named '" + block + "' exists for shader '" + getName() + "'");
    }
    VariableBlockPtr  blockPtr = it->second;
    if (blockPtr->variableMap.find(name) == blockPtr->variableMap.end())
    {
        VariablePtr variablePtr = std::make_shared<Variable>(type, name, semantic, value);
        blockPtr->variableMap[name] = variablePtr;
        blockPtr->variableOrder.push_back(variablePtr.get());
    }
}

const Shader::VariableBlock& Shader::getUniformBlock(size_t stage, const string& block) const
{
    const Stage& s = _stages[stage];
    auto it = s.uniforms.find(block);
    if (it == s.uniforms.end())
    {
        throw ExceptionShaderGenError("No uniform block named '" + block + "' exists for shader '" + getName() + "'");
    }
    return *it->second;
}

void Shader::createAppData(const TypeDesc* type, const string& name, const string& semantic)
{
    if (_appData.variableMap.find(name) == _appData.variableMap.end())
    {
        VariablePtr variable = std::make_shared<Variable>(type, name, semantic);
        _appData.variableMap[name] = variable;
        _appData.variableOrder.push_back(variable.get());
    }
}

void Shader::getTopLevelShaderGraphs(ShaderGenerator& /*shadergen*/, std::deque<ShaderGraph*>& graphs) const
{
    graphs.push_back(_rootGraph.get());
}

}
