//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Material.h>

#include <MaterialXCore/Document.h>

namespace MaterialX
{

vector<NodeDefPtr> Material::getShaderNodeDefs(const string&, const string&) const
{
    return {};
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

} // namespace MaterialX
