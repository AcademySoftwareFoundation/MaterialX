#include <MaterialXShaderGen/SgNode.h>
#include <MaterialXShaderGen/ShaderGenerator.h>
#include <MaterialXShaderGen/Registry.h>
#include <MaterialXShaderGen/CustomImpl.h>
#include <MaterialXShaderGen/Util.h>

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
        if (!readFile(ShaderGenerator::findFile(file), contents))
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

    // Check if this node has a custom implementation
    _customImpl = Registry::findImplementation(_nodeDef->getNode(), language, target);

    if (!_customImpl)
    {
        ElementPtr implElem = node->getImplementation(target);
        if (!(implElem && implElem->isA<Implementation>()))
        {
            throw ExceptionShaderGenError("Could not find a valid implementation for node '" + node->getName() + "' for target '" + target + "'");
        }

        ImplementationPtr impl = implElem->asA<Implementation>();
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

} // namespace MaterialX
