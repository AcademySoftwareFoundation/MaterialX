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
    for (InterfaceElementPtr interface : interfaces)
    {
        if (!targetStringsMatch(interface->getTarget(), target) ||
            !isVersionCompatible(interface))
        {
            continue;
        }
        if (!language.empty())
        {
            // If the given interface is an implementation element, as opposed to
            // a node graph, then check for a language string match.
            ImplementationPtr implement = interface->asA<Implementation>();
            if (implement && implement->getLanguage() != language)
            {
                continue;
            }
        }
        return interface;
    }

    return InterfaceElementPtr();
}

vector<ShaderRefPtr> NodeDef::getInstantiatingShaderRefs() const
{
    vector<ShaderRefPtr> shaderRefs;
    for (MaterialPtr mat : getDocument()->getMaterials())
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

ConstNodeDefPtr NodeDef::getDeclaration(const string&) const
{
    return getSelf()->asA<NodeDef>();
}

//
// Implementation methods
//

void Implementation::setNodeDef(ConstNodeDefPtr nodeDef)
{
    if (nodeDef)
    {
        setNodeDefString(nodeDef->getName());
    }
    else
    {
        removeAttribute(NODE_DEF_ATTRIBUTE);
    }
}

NodeDefPtr Implementation::getNodeDef() const
{
    return resolveRootNameReference<NodeDef>(getNodeDefString());
}

ConstNodeDefPtr Implementation::getDeclaration(const string&) const
{
    return getNodeDef();
}

} // namespace MaterialX
