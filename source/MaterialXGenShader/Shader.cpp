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

const string Shader::PIXEL_STAGE = "pixel";
const string Shader::PRIVATE_UNIFORMS = "PrivateUniforms";
const string Shader::PUBLIC_UNIFORMS = "PublicUniforms";

Shader::Shader(const string& name)
    : _name(name)
    , _rootGraph(nullptr)
    , _appData("AppData", "ad")
    , _outputs("Outputs", "op")
{
    // Create pixel stage and make it the active stage
    StagePtr pixelStage = createStage(PIXEL_STAGE);
    _activeStage = pixelStage.get();

    // Create default uniform blocks for pixel stage
    createUniformBlock(PIXEL_STAGE, PRIVATE_UNIFORMS, "prvUniform");
    createUniformBlock(PIXEL_STAGE, PUBLIC_UNIFORMS, "pubUniform");
}

void Shader::initialize(ElementPtr element, ShaderGenerator& shadergen, const GenOptions& options)
{
    // Check if validation step should be performed
    if (options.validate)
    {
        string message;
        bool valid = element->validate(&message);
        if (!valid)
        {
            throw ExceptionShaderGenError("Element is invalid: " + message);
        }
    }

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
                createUniform(PIXEL_STAGE, PUBLIC_UNIFORMS, inputSocket->type, inputSocket->variable, inputSocket->path, EMPTY_STRING, inputSocket->value);
            }
        }
    }

    // Create outputs from the graph interface
    for (ShaderGraphOutputSocket* outputSocket : _rootGraph->getOutputSockets())
    {
        // Create the output
        if (_outputs.variableMap.find(outputSocket->name) == _outputs.variableMap.end())
        {
            VariablePtr variable = Variable::create(outputSocket->type, outputSocket->name, EMPTY_STRING, EMPTY_STRING, nullptr);
            _outputs.variableMap[outputSocket->name] = variable;
            _outputs.variableOrder.push_back(variable.get());
        }
    }
}

Shader::StagePtr Shader::createStage(const string& name)
{
    StagePtr s = std::make_shared<Stage>(name);
    _stages[name] = s;
    return s;
}

Shader::Stage* Shader::getStage(const string& name)
{
    auto it = _stages.find(name);
    if (it == _stages.end())
    {
        throw ExceptionShaderGenError("Stage '" + name + "' doesn't exist in shader '" + getName() + "'");
    }
    return it->second.get();
}

const Shader::Stage* Shader::getStage(const string& name) const
{
    return const_cast<Shader*>(this)->getStage(name);
}

void Shader::getStageNames(StringVec& stages)
{
    stages.reserve(_stages.size());
    for (auto it : _stages)
    {
        stages.push_back(it.first);
    }
}

void Shader::setActiveStage(const string& name)
{
    _activeStage = getStage(name);
}

void Shader::beginScope(Brackets brackets)
{
    switch (brackets) {
    case Brackets::BRACES:
        indent();
        _activeStage->code += "{\n";
        break;
    case Brackets::PARENTHESES:
        indent();
        _activeStage->code += "(\n";
        break;
    case Brackets::SQUARES:
        indent();
        _activeStage->code += "[\n";
        break;
    case Brackets::NONE:
        break;
    }

    ++_activeStage->indentations;
    _activeStage->scopes.push(brackets);
}

void Shader::endScope(bool semicolon, bool newline)
{
    if (_activeStage->scopes.empty())
    {
        throw ExceptionShaderGenError("End scope called with no scope active, please check your beginScope/endScope calls");
    }

    Brackets brackets = _activeStage->scopes.back();
    _activeStage->scopes.pop();
    --_activeStage->indentations;

    switch (brackets) {
    case Brackets::BRACES:
        indent();
        _activeStage->code += "}";
        break;
    case Brackets::PARENTHESES:
        indent();
        _activeStage->code += ")";
        break;
    case Brackets::SQUARES:
        indent();
        _activeStage->code += "]";
        break;
    case Brackets::NONE:
        break;
    }
    if (semicolon)
        _activeStage->code += ";";
    if (newline)
        _activeStage->code += "\n";
}

void Shader::beginLine()
{
    indent();
}

void Shader::endLine(bool semicolon)
{
    _activeStage->code += semicolon ? ";\n" : "\n";
}

void Shader::newLine()
{
    _activeStage->code += "\n";
}

void Shader::addStr(const string& str)
{
    _activeStage->code += str;
}

void Shader::addLine(const string& str, bool semicolon)
{
    beginLine();
    _activeStage->code += str;
    endLine(semicolon);
}

void Shader::addComment(const string& str)
{
    beginLine();
    _activeStage->code += "// " + str;
    endLine(false);
}

void Shader::addBlock(const string& str, ShaderGenerator& shadergen)
{
    static const string INCLUDE_PATTERN = "#include";
    static const string QUOTATION_MARK = "\"";

    // Add each line in the block seperatelly
    // to get correct indentation
    std::stringstream stream(str);
    for (string line; std::getline(stream, line); )
    {
        size_t pos = line.find(INCLUDE_PATTERN);
        if (pos != string::npos)
        {
            size_t startQuote = line.find_first_of(QUOTATION_MARK);
            size_t endQuote = line.find_last_of(QUOTATION_MARK);
            if (startQuote != string::npos && endQuote != string::npos)
            {
                size_t length = (endQuote - startQuote) - 1;
                if (length)
                {
                    const string filename = line.substr(startQuote + 1, length);
                    addInclude(filename, shadergen);
                }
            }
        }
        else
        {
            addLine(line, false);
        }
    }
}

void Shader::addFunctionDefinition(ShaderNode* node, ShaderGenerator& shadergen)
{
    ShaderNodeImpl* impl = node->getImplementation();
    if (_activeStage->definedFunctions.find(impl) == _activeStage->definedFunctions.end())
    {
        _activeStage->definedFunctions.insert(impl);
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

    if (_activeStage->includes.find(path) == _activeStage->includes.end())
    {
        string content;
        if (!readFile(path, content))
        {
            throw ExceptionShaderGenError("Could not find include file: '" + file + "'");
        }
        _activeStage->includes.insert(path);
        addBlock(content, shadergen);
    }
}

void Shader::indent()
{
    // Use 4 spaces as default indentation
    static const string INDENTATION = "    ";
    for (int i = 0; i < _activeStage->indentations; ++i)
    {
        _activeStage->code += INDENTATION;
    }
}

void Shader::createConstant(const string& stage, const TypeDesc* type, const string& name, const string& path, const string& semantic, ValuePtr value)
{
    Stage* s = getStage(stage);
    if (s->constants.variableMap.find(name) == s->constants.variableMap.end())
    {
        VariablePtr variablePtr = Variable::create(type, name, path, semantic, value);
        s->constants.variableMap[name] = variablePtr;
        s->constants.variableOrder.push_back(variablePtr.get());
    }
}

const Shader::VariableBlock& Shader::getConstantBlock(const string& stage) const
{
    return getStage(stage)->constants;
}

const Shader::VariableBlockMap& Shader::getUniformBlocks(const string& stage) const
{
    return getStage(stage)->uniforms;
}

void Shader::createUniformBlock(const string& stage, const string& block, const string& instance)
{
    Stage* s = getStage(stage);
    auto it = s->uniforms.find(block);
    if (it == s->uniforms.end())
    {
        s->uniforms[block] = std::make_shared<VariableBlock>(block, instance);
    }
}

void Shader::createUniform(const string& stage, const string& block, const TypeDesc* type, const string& name, const string& path, const string& semantic, ValuePtr value)
{
    Stage* s = getStage(stage);
    auto it = s->uniforms.find(block);
    if (it == s->uniforms.end())
    {
        throw ExceptionShaderGenError("No uniform block named '" + block + "' exists for shader '" + getName() + "'");
    }
    VariableBlockPtr  blockPtr = it->second;
    if (blockPtr->variableMap.find(name) == blockPtr->variableMap.end())
    {
        VariablePtr variablePtr = Variable::create(type, name, path, semantic, value);
        blockPtr->variableMap[name] = variablePtr;
        blockPtr->variableOrder.push_back(variablePtr.get());
    }
}

const Shader::VariableBlock& Shader::getUniformBlock(const string& stage, const string& block) const
{
    const Stage* s = getStage(stage);
    auto it = s->uniforms.find(block);
    if (it == s->uniforms.end())
    {
        throw ExceptionShaderGenError("No uniform block named '" + block + "' exists for shader '" + getName() + "'");
    }
    return *it->second;
}

void Shader::createAppData(const TypeDesc* type, const string& name, const string& semantic)
{
    if (_appData.variableMap.find(name) == _appData.variableMap.end())
    {
        VariablePtr variable = Variable::create(type, name, EMPTY_STRING, semantic, nullptr);
        _appData.variableMap[name] = variable;
        _appData.variableOrder.push_back(variable.get());
    }
}

void Shader::getTopLevelShaderGraphs(ShaderGenerator& /*shadergen*/, std::deque<ShaderGraph*>& graphs) const
{
    graphs.push_back(_rootGraph.get());
}

}
