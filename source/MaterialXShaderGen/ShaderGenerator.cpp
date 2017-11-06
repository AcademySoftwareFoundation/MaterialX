#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/SgImplementation.h>
#include <MaterialXShaderGen/Implementations/SourceCode.h>

#include <MaterialXCore/Node.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>

#include <MaterialXFormat/File.h>

#include <sstream>

namespace MaterialX
{

FileSearchPath ShaderGenerator::_sourceCodeSearchPath;

Shader::VDirection ShaderGenerator::getTargetVDirection() const
{
    // Default is to use vdirection up
    return Shader::VDirection::UP;
}

void ShaderGenerator::emitTypeDefs(Shader& shader)
{
    // Emit typedefs for all data types that needs it
    for (auto syntax : _syntax->getTypeSyntax())
    {
        if (syntax.typeDef.length())
        {
            shader.addLine(syntax.typeDef, false);
        }
    }
    shader.newLine();
}

void ShaderGenerator::emitFunctions(Shader& shader)
{
    // Emit funtion definitions for all nodes
    StringSet emittedNodeDefs;
    for (const SgNodePtr& node : shader.getNodes())
    {
        if (emittedNodeDefs.find(node->getNodeDef().getName()) == emittedNodeDefs.end())
        {
            node->getImplementation()->emitFunction(*node, *this, shader);
            emittedNodeDefs.insert(node->getNodeDef().getName());
        }
    }
}

void ShaderGenerator::emitShaderBody(Shader &shader)
{
    const bool debugOutput = true;

    // Emit function calls for all nodes
    for (const SgNodePtr& node : shader.getNodes())
    {
        // Omit node if it's only used inside a conditional branch
        if (node->referencedConditionally())
        {
            if (debugOutput)
            {
                std::stringstream str;
                str << "// Omitted node '" << node->getName() << "'. Only used in conditional node '" << node->getScopeInfo().conditionalNode->getName() << "'";
                shader.addLine(str.str(), false);
            }
            // Omit this node
            continue;
        }

        node->getImplementation()->emitFunctionCall(*node, *this, shader);
    }

    emitFinalOutput(shader);
}

void ShaderGenerator::emitFinalOutput(Shader& shader) const
{
    const OutputPtr& output = shader.getOutput();
    const NodePtr connectedNode = output->getConnectedNode();

    string finalResult = _syntax->getVariableName(*connectedNode);
    if (output->getChannels() != EMPTY_STRING)
    {
        finalResult = _syntax->getSwizzledVariable(finalResult, output->getType(), connectedNode->getType(), output->getChannels());
    }

    shader.addLine(_syntax->getVariableName(*output) + " = " + finalResult);
}

void ShaderGenerator::emitUniform(const string& name, const string& type, const ValuePtr& value, Shader& shader)
{
    const string initStr = (value ? _syntax->getValue(*value, true) : _syntax->getTypeDefault(type, true));
    shader.addStr(_syntax->getTypeName(type) + " " + name + (initStr.empty() ? "" : " = " + initStr));
}

void ShaderGenerator::emitInput(const ValueElement& port, Shader &shader)
{
    if (port.isA<Input>())
    {
        ConstInputPtr input = port.asA<Input>();
        const NodePtr connectedNode = input->getConnectedNode();
        if (connectedNode)
        {
            string name = _syntax->getVariableName(*connectedNode);
            if (input->getChannels() != EMPTY_STRING)
            {
                name = _syntax->getSwizzledVariable(name, input->getType(), connectedNode->getType(), input->getChannels());
            }

            shader.addStr(name);

            return;
        }
    }

    if (!port.getInterfaceName().empty())
    {
        shader.addStr(port.getInterfaceName());
    }
    else if (!port.getPublicName().empty())
    {
        shader.addStr(port.getPublicName());
    }
    else
    {
        const string& valueStr = port.getValueString();
        if (valueStr.length())
        {
            ValuePtr value = port.getValue();
            if (!value)
            {
                throw ExceptionShaderGenError("Malformed value on node port " + port.getParent()->getName() + "." + port.getName());
            }
            shader.addStr(_syntax->getValue(*value));
        }
        else
        {
            shader.addStr(_syntax->getTypeDefault(port.getType()));
        }
    }
}

void ShaderGenerator::emitOutput(const TypedElement& nodeOrOutput, bool includeType, Shader& shader)
{
    string typeStr;
    if (includeType)
    {
        typeStr = _syntax->getTypeName(nodeOrOutput.getType()) + " ";
    }
    shader.addStr(typeStr + _syntax->getVariableName(nodeOrOutput));
}

string ShaderGenerator::id(const string& language, const string& target)
{
    return language + "_" + target;
}

void ShaderGenerator::registerImplementation(const string& name, CreatorFunc<SgImplementation> creator)
{
    _implFactory.registerClass(name, creator);
}

SgImplementationPtr ShaderGenerator::getImplementation(const Implementation& implElement)
{
    const string& name = implElement.getName();

    // Check if it's created already
    auto it = _cachedImpls.find(name);
    if (it != _cachedImpls.end())
    {
        return it->second;
    }

    // Try creating a new in the factory
    SgImplementationPtr impl = _implFactory.create(name);
    if (!impl)
    {
        // No implementation was registed for this name
        // Fall back to data driven source code implementation
        impl = SourceCode::creator();
    }

    impl->initialize(implElement);
    _cachedImpls[name] = impl;

    return impl;
}

void ShaderGenerator::registerSourceCodeSearchPath(const FilePath& path)
{
    _sourceCodeSearchPath.append(path);
}

/// Resolve a file using the registered search paths.
FilePath ShaderGenerator::findSourceCode(const FilePath& filename)
{
    return _sourceCodeSearchPath.find(filename);
}

}
