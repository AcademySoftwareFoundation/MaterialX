#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/Syntax.h>
#include <MaterialXShaderGen/Util.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/Compare.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/Switch.h>

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
    createUniformBlock(PIXEL_STAGE, PRIVATE_UNIFORMS, "prv");
    createUniformBlock(PIXEL_STAGE, PUBLIC_UNIFORMS, "pub");
}

void Shader::initialize(ElementPtr element, ShaderGenerator& shadergen)
{
    // Create our shader generation root graph
    _rootGraph = SgNodeGraph::creator(_name, element, shadergen);

    pushActiveGraph(_rootGraph.get());

    // Set the vdirection to use for texture nodes
    // Default is to use direction UP
    const string& vdir = element->getRoot()->getAttribute("vdirection");
    _vdirection = vdir == "down" ? VDirection::DOWN : VDirection::UP;

    // Create shader variables for all nodes that need this (geometric nodes / input streams)
    for (SgNode* node : _rootGraph->getNodes())
    {
        SgImplementation* impl = node->getImplementation();
        impl->createVariables(*node, shadergen, *this);
    }

    // Create uniforms for the public graph interface
    for (SgInputSocket* inputSocket : _rootGraph->getInputSockets())
    {
        const string name = shadergen.getSyntax()->getVariableName(inputSocket);
        createUniform(PIXEL_STAGE, PUBLIC_UNIFORMS, inputSocket->type, name, EMPTY_STRING, inputSocket->value);
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

void Shader::addBlock(const string& str)
{
    // Add each line in the block seperatelly
    // to get correct indentation
    std::stringstream stream(str);
    for (string line; std::getline(stream, line); )
    {
        addLine(line, false);
    }
}

void Shader::addFunctionDefinition(SgNode* node, ShaderGenerator& shadergen)
{
    Stage& s = stage();
    SgImplementation* impl = node->getImplementation();
    if (s.definedFunctions.find(impl) == s.definedFunctions.end())
    {
        s.definedFunctions.insert(impl);
        impl->emitFunctionDefinition(*node, shadergen, *this);
    }
}

void Shader::addFunctionCall(SgNode* node, ShaderGenerator& shadergen)
{
    SgImplementation* impl = node->getImplementation();
    impl->emitFunctionCall(*node, shadergen, *this);
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
        addBlock(content);
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

void Shader::createUniformBlock(size_t stage, const string& block, const string& instance)
{
    Stage& s = _stages[stage];
    auto it = s.uniforms.find(block);
    if (it == s.uniforms.end())
    {
        s.uniforms[block] = std::make_shared<VariableBlock>(block, instance);
    }
}

void Shader::createUniform(size_t stage, const string& block, const string& type, const string& name, const string& semantic, ValuePtr value)
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

void Shader::createAppData(const string& type, const string& name, const string& semantic)
{
    if (_appData.variableMap.find(name) == _appData.variableMap.end())
    {
        VariablePtr variable = std::make_shared<Variable>(type, name, semantic);
        _appData.variableMap[name] = variable;
        _appData.variableOrder.push_back(variable.get());
    }
}

}
