#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenRegistry.h>
#include <MaterialXShaderGen/NodeImplementation.h>
#include <MaterialXShaderGen/Util.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Value.h>
#include <MaterialXFormat/File.h>

#include <sstream>

namespace MaterialX
{

namespace
{
    static const SgNode EMPTY_SGNODE(nullptr, EMPTY_STRING, EMPTY_STRING);
}

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
    // Emit source code for handling texture coords v-flip 
    // as needed by the v-direction set by the user
    if (shader.getRequestedVDirection() != getTargetVDirection())
    {
        NodeImplementationPtr impl = ShaderGenRegistry::findNodeImplementation("vdirection_flip", getLanguage(), getTarget());
        if (!impl)
        {
           throw ExceptionShaderGenError("Built-in implementation for 'vdirection_flip' was not found. Did you forget to register built-in implementations?");
        }
        impl->emitCode(EMPTY_SGNODE, *this, shader);
    }
    else
    {
        NodeImplementationPtr impl = ShaderGenRegistry::findNodeImplementation("vdirection_noop", getLanguage(), getTarget());
        if (!impl)
        {
           throw ExceptionShaderGenError("Built-in implementation for 'vdirection_noop' was not found. Did you forget to register built-in implementations?");
        }
        impl->emitCode(EMPTY_SGNODE, *this, shader);
    }

    shader.newLine();

    // Emit funtion source code for all nodes that are not inlined
    StringSet emittedFunctions;
    for (const SgNode& node : shader.getNodes())
    {
        if (!node.getInlined() && emittedFunctions.find(node.getFunctionName()) == emittedFunctions.end())
        {
            emitFunction(node, shader);
            emittedFunctions.insert(node.getFunctionName());
        }
    }
}

void ShaderGenerator::emitFunction(const SgNode& node, Shader &shader)
{
    static const string kIncludePattern = "#include ";

    std::stringstream stream(node.getFunctionSource());
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

void ShaderGenerator::emitFunctionCall(const SgNode& node, Shader &shader)
{
    // Check if this node has a custom implementation
    NodeImplementationPtr customImpl = node.getCustomImpl();
    if (customImpl)
    {
        customImpl->emitCode(node, *this, shader);
        return;
    }

    if (node.getInlined())
    {
        // An inline function call

        static const string prefix("{{");
        static const string postfix("}}");

        const string& source = node.getFunctionSource();

        // Inline expressions can only have a single output
        shader.beginLine();
        emitOutput(node.getNode(), true, shader);
        shader.addStr(" = ");

        size_t pos = 0;
        size_t i = source.find_first_of(prefix);
        while (i != string::npos)
        {
            shader.addStr(source.substr(pos, i - pos));

            size_t j = source.find_first_of(postfix, i + 2);
            if (j == string::npos)
            {
                throw ExceptionShaderGenError("Malformed inline expression in implementation for node " + node.getName());
            }

            const string variable = source.substr(i + 2, j - i - 2);
            const ValueElement& port = node.getPort(variable);
            emitInput(port, shader);

            pos = j + 2;
            i = source.find_first_of(prefix, pos);
        }
        shader.addStr(source.substr(pos));
        shader.endLine();
    }
    else
    {
        // An ordinary source code function call
        // TODO: Support multiple outputs

        // Declare the output variable
        shader.beginLine();
        emitOutput(node.getNode(), true, shader);
        shader.endLine();

        shader.beginLine();

        // Emit function name
        shader.addStr(node.getFunctionName() + "(");

        // Emit function inputs
        string delim = "";
        for (ValueElementPtr port : node.getNodeDef().getChildrenOfType<ValueElement>())
        {
            // Find the input port on the node instance
            ValueElementPtr input = node.getNode().getChildOfType<ValueElement>(port->getName());
            if (!input)
            {
                // Not found so used default on the node def
                input = port;
            }

            shader.addStr(delim);
            emitInput(*input, shader);
            delim = ", ";
        }

        // Emit function output
        shader.addStr(delim);
        emitOutput(node.getNode(), false, shader);

        // End function call
        shader.addStr(")");
        shader.endLine();
    }
}

void ShaderGenerator::emitShaderBody(Shader &shader)
{
    const bool debugOutput = true;

    // Emit function calls for all nodes
    for (const SgNode& node : shader.getNodes())
    {
#if 0
        // Omit node if it's only used inside a conditional branch
        const ScopeInfo& scope = cgi.getScopeInfo(childNode->getName());
        if (scope.type == ScopeInfo::Type::SINGLE)
        {
            const NodeClass* nodeClass = childNode->getNodeClass();
            int numBranches = 0;
            uint32_t mask = scope.conditionBitmask;
            for (int j = 0; mask != 0; j++, mask >>= 1)
            {
                if (mask & 1)
                {
                    numBranches++;
                }
            }
            if (numBranches > 0)
            {
                if (debugOutput)
                {
                    std::stringstream str;
                    str << "// Omitted node '" << childNode->getName().str() << "' of class '" << nodeClass->getName().c_str() << "'" << ". Used in branch ";
                    mask = scope.conditionBitmask;
                    string delim = "";
                    for (int j = 0; mask != 0; j++, mask >>= 1)
                    {
                        if (mask & 1)
                        {
                            str << delim << j;
                            delim = ", ";
                        }
                    }
                    shader.addLine(str.str(), false);
                }
                // Omit this node
                continue;
            }
        }
#endif

        emitFunctionCall(node, shader);
    }

    emitFinalOutput(shader);
}

void ShaderGenerator::emitFinalOutput(Shader& shader) const
{
    const OutputPtr& output = shader.getOutput();
    const NodePtr connectedNode = output->getConnectedNode();

    string finalResult = _syntax->getVariableName(*connectedNode);

    const string& outputType = output->getType();
    if (outputType == kSURFACE)
    {
        finalResult = finalResult + ".bsdf + " + finalResult + ".edf";
        string outputExpr = "vec4(pow(" + finalResult + ", vec3(1.0/2.2)), 1.0)";
        shader.addLine(_syntax->getVariableName(*output) + " = " + outputExpr);
    }
    else
    {
        if (output->getChannels() != EMPTY_STRING)
        {
            finalResult = _syntax->getSwizzledVariable(finalResult, output->getType(), connectedNode->getType(), output->getChannels());
        }

        const string typeName = _syntax->getTypeName(outputType);
        string outputExpr = typeName + "(pow(" + finalResult + ", " + typeName + "(1.0/2.2)))";
        shader.addLine(_syntax->getVariableName(*output) + " = " + outputExpr);
    }
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
            bool longName = true;
            string name = _syntax->getVariableName(*connectedNode, longName);

            if (input->getChannels() != EMPTY_STRING)
            {
                name = _syntax->getSwizzledVariable(name, input->getType(), connectedNode->getType(), input->getChannels());
            }

            shader.addStr(name);

            return;
        }
    }

    const string& valueStr = port.getValueString();
    if (valueStr.length())
    {
        ValuePtr value = port.getValue();
        shader.addStr(_syntax->getValue(*value));
    }
    else
    {
        shader.addStr(_syntax->getTypeDefault(port.getType()));
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

}
