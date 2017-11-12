#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/Syntax.h>
#include <MaterialXShaderGen/Util.h>
#include <MaterialXShaderGen/Implementations/Compare.h>
#include <MaterialXShaderGen/Implementations/Switch.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Value.h>

#include <sstream>

namespace MaterialX
{

Shader::Shader(const string& name)
    : _name(name)
    , _activeStage(0)
    , _rootGraph(nullptr)
{
    _stages.resize(numStages());
}

void Shader::initialize(ElementPtr element, ShaderGenerator& shadergen)
{
    _activeStage = 0;
    _stages.resize(numStages());

    // Create our shader generation root graph
    _rootGraph = SgNodeGraph::creator(_name, element, shadergen);

    // Make sure we have a connection inside the graph
    SgOutputSocket* outputSocket = _rootGraph->getOutputSocket();
    if (!outputSocket->connection)
    {
        ExceptionShaderGenError("Graph created for element '" + element->getName() + "' has no internal output connected");
    }

    pushActiveGraph(_rootGraph.get());

    // Set the vdirection to use for texture nodes
    // Default is to use direction UP
    const string& vdir = element->getRoot()->getAttribute("vdirection");
    _vdirection = vdir == "down" ? VDirection::DOWN : VDirection::UP;;

    // Store the graph input sockets as shader uniforms
    for (SgInputSocket* inputSocket : _rootGraph->getInputSockets())
    {
        _uniforms.push_back(Uniform(shadergen.getSyntax()->getVariableName(inputSocket), inputSocket));
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
    SgImplementation* impl = node->getImplementation();
    if (_definedFunctions.count(impl) == 0)
    {
        _definedFunctions.insert(impl);
        impl->emitFunction(*node, shadergen, *this);
    }
}

void Shader::addFunctionCall(SgNode* node, ShaderGenerator& shadergen)
{
    SgImplementation* impl = node->getImplementation();
    impl->emitFunctionCall(*node, shadergen, *this);
}

void Shader::addInclude(const string& file)
{
    const string path = ShaderGenerator::findSourceCode(file);

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
    static const string kIndent = "    ";
    Stage& s = stage();
    for (int i = 0; i < s.indentations; ++i)
    {
        s.code += kIndent;
    }
}

/*
NodePtr Shader::optimize(const Edge& edge)
{
    // Find the downstream element in the new graph.
    ElementPtr downstreamElement = edge.getDownstreamElement();
    ElementPtr downstreamElementNew = _nodeGraph->getChild(getLongName(downstreamElement));
    if (!downstreamElementNew)
    {
        // Downstream element has been pruned
        // so ignore this edge.
        return nullptr;
    }

    // Find the upstream element. If it's an output move on
    // to the actual node connected to the output.
    ElementPtr upstreamElement = edge.getUpstreamElement();
    if (upstreamElement->isA<Output>())
    {
        upstreamElement = upstreamElement->asA<Output>()->getConnectedNode();
        if (!upstreamElement)
        {
            return nullptr;
        }
    }

    // Check if this is a connection to the graph interface
    if (upstreamElement->getParent()->isA<NodeDef>())
    {
        ValueElementPtr interfacePort = upstreamElement->asA<ValueElement>();
        _usedInterface.insert(interfacePort);
        return nullptr;
    }

    NodePtr node = upstreamElement->asA<Node>();
    if (!node)
    {
        return nullptr;
    }

    ValuePtr value = nullptr;
    string publicname = "";

    if (node->getCategory() == "compare")
    {
        // Check if we have a constant conditional expression
        const InputPtr intest = node->getInput("intest");
        NodePtr intestNode = intest->getConnectedNode();
        if (!intestNode || intestNode->getCategory() == "constant")
        {
            const ParameterPtr cutoff = node->getParameter("cutoff");

            const float intestValue = intestNode ?
                intestNode->getParameter("value")->getValue()->asA<float>() :
                intest->getValue()->asA<float>();

            const int branch = (intestValue <= cutoff->getValue()->asA<float>() ? 1 : 2);
            const InputPtr input = node->getInput(Compare::kInputNames[branch]);

            node = input->getConnectedNode();
            if (!node)
            {
                value = input->getValue();
            }
        }
    }
    else if (node->getCategory() == "switch")
    {
        // For switch the conditional is always constant
        const ParameterPtr which = node->getParameter("which");
        const float whichValue = which->getValue()->asA<float>();

        const int branch = int(whichValue);
        const InputPtr input = node->getInput(Switch::kInputNames[branch]);

        node = input->getConnectedNode();
        if (!node)
        {
            value = input->getValue();
        }
    }

    if (node && node->getCategory() == "constant")
    {
        const ParameterPtr param = node->getParameter("value");
        value = param->getValue();
        publicname = param->getPublicName();
    }

    if (value)
    {
        NodePtr downstreamNode = downstreamElementNew->asA<Node>();
        ElementPtr connectingElement = edge.getConnectingElement();
        if (downstreamNode && connectingElement)
        {
            InputPtr input = downstreamNode->getInput(connectingElement->getName());
            if (input)
            {
                input->setConnectedNode(nullptr);
                input->setValueString(value->getValueString());
                input->setPublicName(publicname);
            }
        }

        return nullptr;
    }

    return node;
}
*/

}
