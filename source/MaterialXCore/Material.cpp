//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Material.h>

#include <MaterialXCore/Document.h>

namespace MaterialX
{

const string BindInput::NODE_GRAPH_ATTRIBUTE = "nodegraph";
const string BindInput::OUTPUT_ATTRIBUTE = "output";

const string ShaderRef::NODE_ATTRIBUTE = "node";
const string ShaderRef::NODE_DEF_ATTRIBUTE = "nodedef";

//
// Material methods
//

ShaderRefPtr Material::addShaderRef(const string& name, const string& node)
{
    ShaderRefPtr shaderRef = addChild<ShaderRef>(name);
    if (!node.empty())
    {
        shaderRef->setNodeString(node);
    }
    return shaderRef;
}

vector<NodeDefPtr> Material::getShaderNodeDefs(const string& target, const string& type) const
{
    vector<NodeDefPtr> nodeDefs;
    for (ShaderRefPtr shaderRef : getShaderRefs())
    {
        NodeDefPtr nodeDef = shaderRef->getNodeDef();
        if (!nodeDef || !targetStringsMatch(nodeDef->getTarget(), target))
        {
            continue;
        }
        if (!type.empty() && type != nodeDef->getType())
        {
            continue;
        }
        nodeDefs.push_back(nodeDef);
    }
    return nodeDefs;
}

vector<MaterialAssignPtr> Material::getReferencingMaterialAssigns() const
{
    vector<MaterialAssignPtr> matAssigns;
    for (LookPtr look : getDocument()->getLooks())
    {
        for (MaterialAssignPtr matAssign : look->getMaterialAssigns())
        {
            if (matAssign->getReferencedMaterial() == getSelf())
            {
                matAssigns.push_back(matAssign);
            }
        }
    }
    return matAssigns;
}

void Material::setInheritsFrom(MaterialPtr mat)
{
    for (MaterialInheritPtr inherit : getMaterialInherits())
    {
        removeMaterialInherit(inherit->getName());
    }
    if (mat)
    {
        addMaterialInherit(mat->getName());
    }
}

MaterialPtr Material::getInheritsFrom() const
{
    vector<MaterialInheritPtr> inherits = getMaterialInherits();
    if (inherits.empty())
    {
        return MaterialPtr();
    }
    return getRoot()->getChildOfType<Material>(inherits[0]->getName());
}

vector<ParameterPtr> Material::getPrimaryShaderParameters(const string& target, const string& type) const
{
    NodeDefPtr nodeDef = getPrimaryShaderNodeDef(target, type);
    vector<ParameterPtr> res;
    if (nodeDef)
    {
        InterfaceElementPtr implement = nodeDef->getImplementation();
        for (ParameterPtr nodeDefParam : nodeDef->getParameters())
        {
            ParameterPtr implementParam = implement ? implement->getParameter(nodeDefParam->getName()) : nullptr;
            res.push_back(implementParam ? implementParam : nodeDefParam);
        }
    }
    return res;
}

vector<InputPtr> Material::getPrimaryShaderInputs(const string& target, const string& type) const
{
    NodeDefPtr nodeDef = getPrimaryShaderNodeDef(target, type);
    vector<InputPtr> res;
    if (nodeDef)
    {
        InterfaceElementPtr implement = nodeDef->getImplementation();
        for (InputPtr nodeDefInput : nodeDef->getInputs())
        {
            InputPtr implementInput = implement ? implement->getInput(nodeDefInput->getName()) : nullptr;
            res.push_back(implementInput ? implementInput : nodeDefInput);
        }
    }
    return res;
}

vector<string> Material::getBoundGeomStrings() const
{
    vector<string> geomStrings;
    for (MaterialAssignPtr matAssign : getReferencingMaterialAssigns())
    {
        if (matAssign->hasGeom())
        {
            geomStrings.push_back(matAssign->getGeom());
        }
    }
    return geomStrings;
}

vector<CollectionPtr> Material::getBoundGeomCollections() const
{
    vector<CollectionPtr> collections;
    for (MaterialAssignPtr matAssign : getReferencingMaterialAssigns())
    {
        CollectionPtr collection = matAssign->getCollection();
        if (collection)
        {
            collections.push_back(collection);
        }
    }
    return collections;
}

bool Material::validate(string* message) const
{
    bool res = true;
    validateRequire(!getShaderRefs().empty(), res, message, "Missing shader reference");
    return Element::validate(message) && res;
}

//
// BindInput methods
//

void BindInput::setConnectedOutput(OutputPtr output)
{
    if (output)
    {
        setNodeGraphString(output->getParent()->getName());
        setOutputString(output->getName());
    }
    else
    {
        removeAttribute(NODE_GRAPH_ATTRIBUTE);
        removeAttribute(OUTPUT_ATTRIBUTE);
    }
}

OutputPtr BindInput::getConnectedOutput() const
{
    NodeGraphPtr nodeGraph = getRoot()->getChildOfType<NodeGraph>(getNodeGraphString());
    if (nodeGraph)
    {
        OutputPtr output = nodeGraph->getOutput(getOutputString());
        if (output)
        {
            return output;
        }
    }
    return OutputPtr();
}

//
// ShaderRef methods
//

NodeDefPtr ShaderRef::getNodeDef() const
{
    if (hasNodeDefString())
    {
        return getDocument()->getNodeDef(getNodeDefString());
    }
    if (hasNodeString())
    {
        vector<NodeDefPtr> nodeDefs = getDocument()->getMatchingNodeDefs(getNodeString());
        return nodeDefs.empty() ? NodeDefPtr() : nodeDefs[0];
    }
    return NodeDefPtr();
}

bool ShaderRef::validate(string* message) const
{
    bool res = true;
    NodeDefPtr nodeDef = getNodeDef();
    TypeDefPtr typeDef = nodeDef ? getDocument()->getTypeDef(nodeDef->getType()) : TypeDefPtr();
    if (typeDef)
    {
        validateRequire(typeDef->getSemantic() == SHADER_SEMANTIC, res, message, "Shader reference to a non-shader nodedef");
    }
    return Element::validate(message) && res;
}

//
// Override methods
//

ConstElementPtr Override::getReceiver() const
{
    return getDocument()->getPublicElement(getName());
}

} // namespace MaterialX
