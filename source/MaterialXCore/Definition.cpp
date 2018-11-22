//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Definition.h>

#include <MaterialXCore/Document.h>

namespace MaterialX
{

const string COLOR_SEMANTIC = "color";
const string SHADER_SEMANTIC = "shader";

const string TEXTURE_NODE_GROUP = "texture";
const string PROCEDURAL_NODE_GROUP = "procedural";
const string GEOMETRIC_NODE_GROUP = "geometric";
const string ADJUSTMENT_NODE_GROUP = "adjustment";
const string CONDITIONAL_NODE_GROUP = "conditional";

const string NodeDef::NODE_ATTRIBUTE = "node";
const string NodeDef::NODE_GROUP_ATTRIBUTE = "nodegroup";
const string TypeDef::SEMANTIC_ATTRIBUTE = "semantic";
const string TypeDef::CONTEXT_ATTRIBUTE = "context";
const string Implementation::FILE_ATTRIBUTE = "file";
const string Implementation::FUNCTION_ATTRIBUTE = "function";
const string Implementation::LANGUAGE_ATTRIBUTE = "language";

//
// NodeDef methods
//

InterfaceElementPtr NodeDef::getImplementation(const string& target, const string& language) const
{
    vector<InterfaceElementPtr> interfaces = getDocument()->getMatchingImplementations(getQualifiedName(getName()));
    vector<InterfaceElementPtr> secondary = getDocument()->getMatchingImplementations(getName());
    interfaces.insert(interfaces.end(), secondary.begin(), secondary.end());

    // Search for the first implementation which matches a given language string.
    // If no language is specified then return the first implementation found.
    bool matchLanguage = !language.empty();
    for (InterfaceElementPtr interface : interfaces)
    {
        ImplementationPtr implement = interface->asA<Implementation>();
        if (!implement||
            !targetStringsMatch(interface->getTarget(), target) ||
            !isVersionCompatible(interface))
        {
            continue;
        }
        if (!matchLanguage || 
            implement->getLanguage() == language)
        {
            return interface;
        }
    }

    // Search for a node graph match if no implementation match was found.
    // There is no language check as node graphs are considered to be language independent.
    for (InterfaceElementPtr interface : interfaces)
    {
        if (interface->isA<Implementation>() || 
            !targetStringsMatch(interface->getTarget(), target) ||
            !isVersionCompatible(interface))
        {
            continue;
        }
        return interface;
    }

    return InterfaceElementPtr();
}

vector<ShaderRefPtr> NodeDef::getInstantiatingShaderRefs() const
{
    vector<ShaderRefPtr> shaderRefs;
    for (MaterialPtr mat : getRoot()->getChildrenOfType<Material>())
    {
        for (ShaderRefPtr shaderRef : mat->getShaderRefs())
        {
            if (shaderRef->getNodeDef()->hasInheritedBase(getSelf()))
            {
                shaderRefs.push_back(shaderRef);
            }
        }
    }
    return shaderRefs;
}

bool NodeDef::validate(string* message) const
{
    bool res = true;
    validateRequire(hasType(), res, message, "Missing type");
    if (isMultiOutputType())
    {
        validateRequire(getOutputCount() >= 2, res, message, "Multioutput nodedefs must have two or more output ports");
    }
    else
    {
        validateRequire(getOutputCount() == 0, res, message, "Only multioutput nodedefs support output ports");
    }
    return InterfaceElement::validate(message) && res;
}

bool NodeDef::isVersionCompatible(ConstElementPtr elem) const
{
    if (getVersionIntegers() == elem->getVersionIntegers())
    {
        return true;
    }
    if (getDefaultVersion() && !elem->hasVersionString())
    {
        return true;
    }
    return false;
}

} // namespace MaterialX
