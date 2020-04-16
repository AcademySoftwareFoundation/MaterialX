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

const string NodeDef::TEXTURE_NODE_GROUP = "texture";
const string NodeDef::PROCEDURAL_NODE_GROUP = "procedural";
const string NodeDef::GEOMETRIC_NODE_GROUP = "geometric";
const string NodeDef::ADJUSTMENT_NODE_GROUP = "adjustment";
const string NodeDef::CONDITIONAL_NODE_GROUP = "conditional";
const string NodeDef::ORGANIZATION_NODE_GROUP = "organization";

const string NodeDef::NODE_ATTRIBUTE = "node";
const string NodeDef::NODE_GROUP_ATTRIBUTE = "nodegroup";
const string TypeDef::SEMANTIC_ATTRIBUTE = "semantic";
const string TypeDef::CONTEXT_ATTRIBUTE = "context";
const string Implementation::FILE_ATTRIBUTE = "file";
const string Implementation::FUNCTION_ATTRIBUTE = "function";
const string Implementation::LANGUAGE_ATTRIBUTE = "language";
const string UnitDef::UNITTYPE_ATTRIBUTE = "unittype";
const string AttributeDef::ATTRNAME_ATTRIBUTE = "attrname";
const string AttributeDef::VALUE_ATTRIBUTE = "value";
const string AttributeDef::ELEMENTS_ATTRIBUTE = "elements";
const string AttributeDef::EXPORTABLE_ATTRIBUTE = "exportable";

//
// NodeDef methods
//

const string& NodeDef::getType() const
{
    const vector<OutputPtr>& activeOutputs = getActiveOutputs();

    size_t numActiveOutputs = activeOutputs.size();
    if (numActiveOutputs > 1)
    {
        return MULTI_OUTPUT_TYPE_STRING;
    }
    else if (numActiveOutputs == 1)
    {
        return activeOutputs[0]->getType();
    }
    else
    {
        throw Exception("Nodedef: " + getName() + " has no outputs");
    }
}

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
    validateRequire(!hasType(), res, message, "Nodedef should not have a type but an explicit output");
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

vector<UnitDefPtr> UnitTypeDef::getUnitDefs() const
{
    vector<UnitDefPtr> unitDefs;
    for (UnitDefPtr unitDef : getDocument()->getChildrenOfType<UnitDef>())
    {
        if (unitDef->getUnitType() == _name)
        {
            unitDefs.push_back(unitDef);
        }
    }
    return unitDefs;
}

} // namespace MaterialX
