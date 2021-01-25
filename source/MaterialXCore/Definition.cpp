//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Definition.h>

#include <MaterialXCore/Document.h>

namespace MaterialX
{

namespace
{
    InterfaceElementPtr findMatchingImplementation(const NodeDef& nodedef, const string& target, const vector<InterfaceElementPtr>& candidates)
    {
        // Search for the first implementation which matches the given target string.
        for (InterfaceElementPtr interface : candidates)
        {
            if (!interface->isA<Implementation>() ||
                !targetStringsMatch(interface->getTarget(), target) ||
                !nodedef.isVersionCompatible(interface))
            {
                continue;
            }
            return interface;
        }

        // Search for a node graph match if no implementation match was found.
        for (InterfaceElementPtr interface : candidates)
        {
            if (!interface->isA<NodeGraph>() ||
                !targetStringsMatch(interface->getTarget(), target) ||
                !nodedef.isVersionCompatible(interface))
            {
                continue;
            }
            return interface;
        }

        return InterfaceElementPtr();
    }
}

const string COLOR_SEMANTIC = "color";
const string SHADER_SEMANTIC = "shader";

const string NodeDef::TEXTURE_NODE_GROUP = "texture";
const string NodeDef::PROCEDURAL_NODE_GROUP = "procedural";
const string NodeDef::GEOMETRIC_NODE_GROUP = "geometric";
const string NodeDef::ADJUSTMENT_NODE_GROUP = "adjustment";
const string NodeDef::CONDITIONAL_NODE_GROUP = "conditional";
const string NodeDef::ORGANIZATION_NODE_GROUP = "organization";
const string NodeDef::TRANSLATION_NODE_GROUP = "translation";

const string NodeDef::NODE_ATTRIBUTE = "node";
const string NodeDef::NODE_GROUP_ATTRIBUTE = "nodegroup";
const string TypeDef::SEMANTIC_ATTRIBUTE = "semantic";
const string TypeDef::CONTEXT_ATTRIBUTE = "context";
const string Implementation::FILE_ATTRIBUTE = "file";
const string Implementation::FUNCTION_ATTRIBUTE = "function";
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
        return DEFAULT_TYPE_STRING;
    }
}

InterfaceElementPtr NodeDef::getImplementation(const string& target) const
{
    vector<InterfaceElementPtr> interfaces = getDocument()->getMatchingImplementations(getQualifiedName(getName()));
    vector<InterfaceElementPtr> secondary = getDocument()->getMatchingImplementations(getName());
    interfaces.insert(interfaces.end(), secondary.begin(), secondary.end());

    if (target.empty())
    {
        return findMatchingImplementation(*this, target, interfaces);
    }

    // Get all candidate targets matching the given target,
    // taking inheritance into account.
    const TargetDefPtr targetDef = getDocument()->getTargetDef(target);
    const StringVec candidateTargets = targetDef ? targetDef->getMatchingTargets() : StringVec();

    // Search the candidate targets in order
    for (const string& candidateTarget : candidateTargets)
    {
        InterfaceElementPtr impl = findMatchingImplementation(*this, candidateTarget, interfaces);
        if (impl)
        {
            return impl;
        }
    }

    return InterfaceElementPtr();
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

StringVec TargetDef::getMatchingTargets() const
{
    StringVec result = { getName() };
    ElementPtr base = getInheritsFrom();
    while (base)
    {
        result.push_back(base->getName());
        base = base->getInheritsFrom();
    }
    return result;
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
