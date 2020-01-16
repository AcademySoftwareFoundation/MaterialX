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
const RtToken PvtNodeGraph::_typeName = "nodegraph";

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
        graphName = parent->makeUniqueName(RtToken(_typeName.str() + "1"));
    }

    return PvtDataHandle(new PvtNodeGraph(graphName, parent));
}

PvtAttribute* PvtNodeGraph::createAttribute(const RtToken& name, const RtToken& type, uint32_t flags)
{
    PvtAttribute* attr = PvtPrim::createAttribute(name, type, flags);

    // Toggle input vs output and set socket flag.
    flags ^= RtAttrFlag::OUTPUT;
    flags |= RtAttrFlag::SOCKET;

    PvtDataHandle sockH(new PvtAttribute(name, type, flags, this));
    _socketOrder.push_back(sockH);
    _socketMap[name] = sockH;

    return attr;
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
