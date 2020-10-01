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

vector<ShaderRefPtr> Material::getActiveShaderRefs() const
{
    vector<ShaderRefPtr> activeShaderRefs;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<ShaderRefPtr> shaderRefs = elem->asA<Material>()->getShaderRefs();
        activeShaderRefs.insert(activeShaderRefs.end(), shaderRefs.begin(), shaderRefs.end());
    }
    return activeShaderRefs;
}

vector<NodeDefPtr> Material::getShaderNodeDefs(const string& target, const string& type) const
{
    vector<NodeDefPtr> nodeDefs;
    for (ShaderRefPtr shaderRef : getActiveShaderRefs())
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

vector<MaterialAssignPtr> Material::getGeometryBindings(const string& geom) const
{
    vector<MaterialAssignPtr> matAssigns;
    for (LookPtr look : getDocument()->getLooks())
    {
        for (MaterialAssignPtr matAssign : look->getMaterialAssigns())
        {
            if (matAssign->getReferencedMaterial() == getSelf())
            {
                if (geomStringsMatch(geom, matAssign->getActiveGeom()))
                {
                    matAssigns.push_back(matAssign);
                    continue;
                }
                CollectionPtr coll = matAssign->getCollection();
                if (coll && coll->matchesGeomString(geom))
                {
                    matAssigns.push_back(matAssign);
                    continue;
                }
            }
        }
    }
    return matAssigns;
}

vector<InputPtr> Material::getPrimaryShaderInputs(const string& target, const string& type) const
{
    NodeDefPtr nodeDef = getPrimaryShaderNodeDef(target, type);
    vector<InputPtr> res;
    if (nodeDef)
    {
        InterfaceElementPtr implement = nodeDef->getImplementation();
        for (InputPtr nodeDefInput : nodeDef->getActiveInputs())
        {
            InputPtr implementInput = implement ? implement->getInput(nodeDefInput->getName()) : nullptr;
            res.push_back(implementInput ? implementInput : nodeDefInput);
        }
    }
    return res;
}

vector<TokenPtr> Material::getPrimaryShaderTokens(const string& target, const string& type) const
{
    NodeDefPtr nodeDef = getPrimaryShaderNodeDef(target, type);
    vector<TokenPtr> res;
    if (nodeDef)
    {
        InterfaceElementPtr implement = nodeDef->getImplementation();
        for (TokenPtr nodeDefToken : nodeDef->getActiveTokens())
        {
            TokenPtr implementToken = implement ? implement->getToken(nodeDefToken->getName()) : nullptr;
            res.push_back(implementToken ? implementToken : nodeDefToken);
        }
    }
    return res;
}

bool Material::validate(string* message) const
{
    bool res = true;
    if (!hasInheritanceCycle())
    {
        validateRequire(!getActiveShaderRefs().empty(), res, message, "Missing shader reference");
    }
    return Element::validate(message) && res;
}

//
// BindParam methods
//

bool BindParam::validate(string* message) const
{
    bool res = true;
    ConstElementPtr parent = getParent();
    ConstShaderRefPtr shaderRef = parent ? parent->asA<ShaderRef>() : nullptr;
    NodeDefPtr nodeDef = shaderRef ? shaderRef->getNodeDef() : nullptr;
    if (nodeDef)
    {
        ParameterPtr param = nodeDef->getActiveParameter(getName());
        validateRequire(param != nullptr, res, message, "BindParam does not match a Parameter in the referenced NodeDef");
        if (param)
        {
            validateRequire(getType() == param->getType(), res, message, "Type mismatch between BindParam and Parameter");
        }
    }
    return ValueElement::validate(message) && res;
}

//
// BindInput methods
//

void BindInput::setConnectedOutput(ConstOutputPtr output)
{
    if (output)
    {
        setOutputString(output->getName());
        ConstElementPtr parent = output->getParent();
        if (parent->isA<NodeGraph>())
        {
            setNodeGraphString(parent->getName());
        }
        else
        {
            removeAttribute(NODE_GRAPH_ATTRIBUTE);
        }
    }
    else
    {
        removeAttribute(OUTPUT_ATTRIBUTE);
        removeAttribute(NODE_GRAPH_ATTRIBUTE);
    }
}

OutputPtr BindInput::getConnectedOutput() const
{
    if (hasNodeGraphString())
    {
        NodeGraphPtr nodeGraph = resolveRootNameReference<NodeGraph>(getNodeGraphString());
        return nodeGraph ? nodeGraph->getOutput(getOutputString()) : OutputPtr();
    }
    return getDocument()->getOutput(getOutputString());
}

bool BindInput::validate(string* message) const
{
    bool res = true;
    ConstElementPtr parent = getParent();
    ConstShaderRefPtr shaderRef = parent ? parent->asA<ShaderRef>() : nullptr;
    NodeDefPtr nodeDef = shaderRef ? shaderRef->getNodeDef() : nullptr;
    if (nodeDef)
    {
        InputPtr input = nodeDef->getActiveInput(getName());
        validateRequire(input != nullptr, res, message, "BindInput does not match an Input in the referenced NodeDef");
        if (input)
        {
            validateRequire(getType() == input->getType(), res, message, "Type mismatch between BindInput and Input");
        }
    }
    if (hasNodeGraphString())
    {
        NodeGraphPtr nodeGraph = resolveRootNameReference<NodeGraph>(getNodeGraphString());
        validateRequire(nodeGraph != nullptr, res, message, "Invalid node graph attribute on BindInput");
    }
    if (hasOutputString())
    {
        OutputPtr output = getConnectedOutput();
        validateRequire(output != nullptr, res, message, "Invalid output attribute on BindInput");
    }
    return ValueElement::validate(message) && res;
}

//
// BindToken methods
//

bool BindToken::validate(string* message) const
{
    bool res = true;
    ConstElementPtr parent = getParent();
    ConstShaderRefPtr shaderRef = parent ? parent->asA<ShaderRef>() : nullptr;
    NodeDefPtr nodeDef = shaderRef ? shaderRef->getNodeDef() : nullptr;
    if (nodeDef)
    {
        TokenPtr token = nodeDef->getActiveToken(getName());
        validateRequire(token != nullptr, res, message, "BindToken does not match a Token in the referenced NodeDef");
    }
    return ValueElement::validate(message) && res;
}

//
// ShaderRef methods
//

NodeDefPtr ShaderRef::getNodeDef() const
{
    if (hasNodeDefString())
    {
        return resolveRootNameReference<NodeDef>(getNodeDefString());
    }
    if (hasNodeString())
    {
        vector<NodeDefPtr> nodeDefs = getDocument()->getMatchingNodeDefs(getQualifiedName(getNodeString()));
        vector<NodeDefPtr> secondary = getDocument()->getMatchingNodeDefs(getNodeString());
        nodeDefs.insert(nodeDefs.end(), secondary.begin(), secondary.end());
        for (NodeDefPtr nodeDef : nodeDefs)
        {
            if (targetStringsMatch(nodeDef->getTarget(), getTarget()) &&
                nodeDef->isVersionCompatible(getSelf()) &&
                (!hasType() || nodeDef->getType() == getType()))
            {
                return nodeDef;
            }
        }
    }
    return NodeDefPtr();
}

bool ShaderRef::validate(string* message) const
{
    bool res = true;
    NodeDefPtr nodeDef = getNodeDef();
    TypeDefPtr typeDef = nodeDef ? resolveRootNameReference<TypeDef>(nodeDef->getType()) : TypeDefPtr();
    if (!nodeDef)
    {
        validateRequire(!hasNodeString() && !hasNodeDefString(), res, message, "Shader reference to a non-existent nodedef");
    }
    if (typeDef)
    {
        validateRequire(typeDef->getSemantic() == SHADER_SEMANTIC, res, message, "Shader reference to a non-shader nodedef");
    }
    return TypedElement::validate(message) && res;
}

Edge ShaderRef::getUpstreamEdge(ConstMaterialPtr material, size_t index) const
{
    if (index < getUpstreamEdgeCount())
    {
        BindInputPtr input = getBindInputs()[index];
        ElementPtr upstreamOutput = input->getConnectedOutput();
        if (upstreamOutput)
        {
            return Edge(getSelfNonConst(), input, upstreamOutput);
        }
    }

    return NULL_EDGE;
}

} // namespace MaterialX
