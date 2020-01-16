//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtNodeGraph.h>

#include <MaterialXRuntime/RtTypeDef.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

const RtObjType PvtNodeGraph::_typeId = RtObjType::NODEGRAPH;
const RtToken PvtNodeGraph::_typeName = RtToken("nodegraph");

PvtNodeGraph::PvtNodeGraph(const RtToken& name, PvtPrim* parent) :
    PvtNode(name, parent)
{
}

PvtDataHandle PvtNodeGraph::createNew(const RtToken& name, PvtPrim* parent)
{
    // If a name is not given generate one.
    RtToken graphName = name;
    if (graphName == EMPTY_TOKEN)
    {
        graphName = RtToken(_typeName.str() + "1");
    }

    // Make the name unique.
    // Check for nullptr as the stage root prim is
    // a nodegraph without a parent.
    if (parent)
    {
        graphName = parent->makeUniqueName(graphName);
    }

    return PvtDataHandle(new PvtNodeGraph(graphName, parent));
}

PvtAttribute* PvtNodeGraph::createAttribute(const RtToken& name, const RtToken& type, uint32_t flags)
{
    // Create for the public interface
    // Graph ports should always be connectable.
    flags |= RtAttrFlag::CONNECTABLE;
    PvtAttribute* attr = PvtPrim::createAttribute(name, type, flags);

    // Create for the internal socket interface.
    // Toggle input vs output and set socket flag.
    flags ^= RtAttrFlag::OUTPUT;
    flags |= RtAttrFlag::SOCKET;
    PvtDataHandle sockH(new PvtAttribute(name, type, flags, this));
    _socketOrder.push_back(sockH);
    _socketMap[name] = sockH;

    return attr;
}

PvtAttribute* PvtNodeGraph::getInputSocket(const RtToken& name) const
{
    auto it = _socketMap.find(name);
    if (it != _socketMap.end())
    {
        PvtAttribute* attr = it->second->asA<PvtAttribute>();
        return attr->isOutput() && attr->isConnectable() ? attr : nullptr;
    }
    return nullptr;
}

PvtAttribute* PvtNodeGraph::getOutputSocket(const RtToken& name) const
{
    auto it = _socketMap.find(name);
    if (it != _socketMap.end())
    {
        PvtAttribute* attr = it->second->asA<PvtAttribute>();
        return attr->isInput() && attr->isConnectable() ? attr : nullptr;
    }
    return nullptr;
}

string PvtNodeGraph::asStringDot() const
{
    string dot = "digraph {\n";

    // Add alla nodes.
    dot += "    \"inputs\" ";
    dot += "[shape=box];\n";
    dot += "    \"outputs\" ";
    dot += "[shape=box];\n";

    RtObjTypePredicate<RtObjType::NODE> nodeFilter;

    // Add all nodes.
    for (const RtObject prim : getChildren(nodeFilter))
    {
        dot += "    \"" + PvtObject::ptr<PvtNode>(prim)->getName().str() + "\" ";
        dot += "[shape=ellipse];\n";
    }

    auto writeConnections = [](const PvtNode* node, string& dot)
    {
        string dstName = node->getName().str();
        for (const RtObject attrObj : node->getAttributes())
        {
            const PvtAttribute* attr = PvtObject::ptr<PvtAttribute>(attrObj);
            if (attr->isInput() && attr->isConnected())
            {
                const PvtAttribute* src = attr->getConnection();
                string srcName = src->getParent()->getName().str();
                dot += "    \"" + srcName;
                dot += "\" -> \"" + dstName;
                dot += "\" [label=\"" + attr->getName().str() + "\"];\n";
            }
        }
    };

    // Add all connections.
    for (const RtObject prim : getChildren(nodeFilter))
    {
        writeConnections(PvtObject::ptr<PvtNode>(prim), dot);
    }

    dot += "}\n";

    return dot;
}

}
