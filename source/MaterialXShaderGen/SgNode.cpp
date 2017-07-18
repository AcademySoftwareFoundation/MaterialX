#include <MaterialXShaderGen/SgNode.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenRegistry.h>
#include <MaterialXShaderGen/NodeImplementation.h>
#include <MaterialXShaderGen/Util.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>

#include <MaterialXFormat/File.h>

#include <sstream>

namespace MaterialX
{

namespace
{
    void getSourceCode(const Implementation& impl, string& source, bool& inlined)
    {
        const string& file = impl.getAttribute("file");
        if (file.empty())
        {
            throw ExceptionShaderGenError("No source file specified for implementation '" + impl.getName() + "'");
        }

        source = "";
        inlined = (getFileExtension(file) == "inline");

        // Find the function name to use
        string function = impl.getAttribute("function");
        if (function.empty())
        {
            // No function given so use node def name
            function = impl.getNodeDef();
        }

        string contents;
        if (!readFile(ShaderGenRegistry::findSourceCode(file), contents))
        {
            throw ExceptionShaderGenError("Can't find source file '" + file + "' used by implementation '" + impl.getName() + "'");
        }

        // Get the source code
        if (inlined)
        {
            if (function[0] == '@')
            {
                std::stringstream stream(contents);
                std::string line;
                while (std::getline(stream, line))
                {
                    if (line == function)
                    {
                        std::getline(stream, line);
                        source = line;
                        break;
                    }
                }
            }
            else
            {
                source = contents;
                source.erase(std::remove(source.begin(), source.end(), '\n'), source.end());
            }
        }
        else
        {
            // Find the function source code in the file
            std::stringstream stream(contents);
            bool found = false;
            string line;
            while (std::getline(stream, line))
            {
                if (line.find("#include") != string::npos)
                {
                    source += line + "\n";
                }
                if (!found && line.find(" " + function) != string::npos)
                {
                    found = true;
                }
                if (found)
                {
                    source += line + "\n";
                    if (line == "}")
                    {
                        break;
                    }
                }
            }
            if (!found)
            {
                throw ExceptionShaderGenError("Function '" + function + "' was not found in file '" + file + "' used by implementation '" + impl.getName() + "'");
            }
        }
    }
}

SgNode::SgNode(NodePtr node, const string& language, const string& target)
    : _node(node)
    , _nodeDef(nullptr)
    , _customImpl(nullptr)
{
    if (!_node)
    {
        return;
    }

    _nodeDef = node->getReferencedNodeDef();
    if (!_nodeDef)
    {
        throw ExceptionShaderGenError("Could not find a nodedef for node '" + node->getName() + "'");
    }

    // Check if this node has a custom node implementation registered
    _customImpl = ShaderGenRegistry::findNodeImplementation(_nodeDef->getNode(), language, target);

    if (!_customImpl)
    {
        ImplementationPtr impl = getSourceCodeImplementation(*_nodeDef, language, target);
        if (!impl)
        {
            throw ExceptionShaderGenError("Could not find an implementation for node '" + _nodeDef->getNode() + "' matching language '" + language + "' and target '" + target + "'");
        }

        getSourceCode(*impl, _functionSource, _inlined);
        _functionName = impl->getFunction();
    }
}

const ValueElement& SgNode::getPort(const string& name) const
{
    ValueElementPtr port = _node->getChildOfType<ValueElement>(name);
    if (!port)
    {
        port = _nodeDef->getChildOfType<ValueElement>(name);
        if (!port)
        {
            throw ExceptionShaderGenError("Node '" + _node->getName() + "' has no input port named '" + name + "'");
        }
    }
    return *port;
}

ImplementationPtr SgNode::getSourceCodeImplementation(const NodeDef& nodeDef, const string& language, const string& target)
{
    vector<ElementPtr> elements = nodeDef.getDocument()->getMatchingImplementations(nodeDef.getName());
    for (ElementPtr element : elements)
    {
        if (!element->isA<Implementation>())
        {
            // Ignore node graph implementations
            continue;
        }

        ImplementationPtr impl = element->asA<Implementation>();

        const string& matchingTarget = impl->getTarget();

        if (impl->getLanguage() == language && (matchingTarget.empty() || matchingTarget == target))
        {
            return impl;
        }
    }

    return nullptr;
}

} // namespace MaterialX
