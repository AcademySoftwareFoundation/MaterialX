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

Shader::Shader(const string& name)
    : _name(name)
    , _rootGraph(nullptr)
    , _activeStage(PIXEL_STAGE)
{
    _stages.resize(numStages());
}

void Shader::initialize(ElementPtr element, ShaderGenerator& shadergen)
{
    _activeStage = PIXEL_STAGE;
    _stages.resize(numStages());

    // Create our shader generation root graph
    _rootGraph = SgNodeGraph::creator(_name, element, shadergen);

    pushActiveGraph(_rootGraph.get());

    // Set the vdirection to use for texture nodes
    // Default is to use direction UP
    const string& vdir = element->getRoot()->getAttribute("vdirection");
    _vdirection = vdir == "down" ? VDirection::DOWN : VDirection::UP;

    // Register shader uniforms for the graph interface
    for (SgInputSocket* inputSocket : _rootGraph->getInputSockets())
    {
        const string name = shadergen.getSyntax()->getVariableName(inputSocket);
        registerUniform(Variable(inputSocket->type, name, EMPTY_STRING, inputSocket->value));
    }

    // Add shader variables for nodes that need this (geometric nodes / input streams)
    for (SgNode* node : _rootGraph->getNodes())
    {
        SgImplementation* impl = node->getImplementation();
        impl->registerVariables(*node, shadergen, *this);
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

void Shader::endScope(bool semicolon)
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
    s.code += semicolon ? ";\n" : "\n";
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

void Shader::registerUniform(const Variable& uniform, size_t stage)
{
    Stage& s = _stages[stage];
    if (!s.registeredVariables.count(uniform.name))
    {
        s.registeredVariables.insert(uniform.name);
        s.uniforms.push_back(uniform);
    }
}

void Shader::registerInput(const Variable& input, size_t stage)
{
    Stage& s = _stages[stage];
    if (!s.registeredVariables.count(input.name))
    {
        s.registeredVariables.insert(input.name);
        s.inputs.push_back(input);
    }
}

void Shader::registerOutput(const Variable& output, size_t stage)
{
    Stage& s = _stages[stage];
    if (!s.registeredVariables.count(output.name))
    {
        s.registeredVariables.insert(output.name);
        s.outputs.push_back(output);
    }
}

}
