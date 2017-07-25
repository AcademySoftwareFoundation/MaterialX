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
        shaderRef->setNode(node);
    }
    return shaderRef;
}

vector<NodeDefPtr> Material::getReferencedShaderDefs() const
{
    vector<NodeDefPtr> shaderDefs;
    for (ShaderRefPtr shaderRef : getShaderRefs())
    {
        NodeDefPtr shaderDef = shaderRef->getReferencedShaderDef();
        if (shaderDef)
        {
            shaderDefs.push_back(shaderDef);
        }
    }
    return shaderDefs;
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

NodeDefPtr ShaderRef::getReferencedShaderDef()
{
    if (hasNodeDef())
    {
        return getDocument()->getNodeDef(getNodeDef());
    }
    if (hasNode())
    {
        vector<NodeDefPtr> nodeDefs = getDocument()->getMatchingNodeDefs(getNode());
        return nodeDefs.empty() ? NodeDefPtr() : nodeDefs[0];
    }
    return NodeDefPtr();
}

Edge ShaderRef::getUpstreamEdge(MaterialPtr material, size_t index)
{
    if (index < getUpstreamEdgeCount())
    {
        BindInputPtr input = getBindInputs()[index];
        ElementPtr upstreamOutput = input->getConnectedOutput();
        if (upstreamOutput)
        {
            return Edge(getSelf(), input, upstreamOutput);
        }
    }

    return NULL_EDGE;
}


//
// Override methods
//

ConstElementPtr Override::getReceiver() const
{
    return getDocument()->getPublicElement(getName());
}

} // namespace MaterialX
