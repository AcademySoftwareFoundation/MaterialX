//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/ShaderGraphHash.h>

#include <functional>

MATERIALX_NAMESPACE_BEGIN

namespace
{

void hashCombine(size_t& seed, size_t value)
{
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

void hashString(size_t& seed, const string& str)
{
    hashCombine(seed, std::hash<string>()(str));
}

void hashUint32(size_t& seed, uint32_t value)
{
    hashCombine(seed, std::hash<uint32_t>()(value));
}

void hashSize(size_t& seed, size_t value)
{
    hashCombine(seed, std::hash<size_t>()(value));
}

void hashPortStructure(size_t& seed, const ShaderPort* port)
{
    hashString(seed, port->getType().getName());
    hashString(seed, port->getSemantic());
    hashString(seed, port->getColorSpace());
    hashString(seed, port->getUnit());
    hashString(seed, port->getGeomProp());

    uint32_t structuralFlags = port->getFlags() &
        (ShaderPortFlag::UNIFORM | ShaderPortFlag::BIND_INPUT);
    hashUint32(seed, structuralFlags);
}

size_t findOutputIndex(const ShaderOutput* output)
{
    const ShaderNode* node = output->getNode();
    for (size_t i = 0; i < node->numOutputs(); ++i)
    {
        if (node->getOutput(i) == output)
            return i;
    }
    return SIZE_MAX;
}

} // anonymous namespace

size_t computeStructuralHash(const ShaderGraph& graph)
{
    size_t seed = 0;

    // 1. Hash graph-level input sockets (count + structural type info, no names or values)
    hashSize(seed, graph.numInputSockets());
    for (const ShaderGraphInputSocket* socket : graph.getInputSockets())
    {
        hashPortStructure(seed, socket);
    }

    // 2. Hash graph-level output sockets (count + structural type info)
    hashSize(seed, graph.numOutputSockets());
    for (const ShaderGraphOutputSocket* socket : graph.getOutputSockets())
    {
        hashPortStructure(seed, socket);
    }

    // 3. Build a stable index for each node using topological order
    const auto& nodes = graph.getNodes();
    std::unordered_map<const ShaderNode*, size_t> nodeIndex;
    nodeIndex.reserve(nodes.size() + 1);
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        nodeIndex[nodes[i]] = i;
    }
    // The graph itself can appear as a connection source (for graph input sockets)
    constexpr size_t GRAPH_SELF_INDEX = SIZE_MAX;
    constexpr size_t NO_CONNECTION_SENTINEL = SIZE_MAX - 1;
    constexpr size_t UNKNOWN_NODE_SENTINEL = SIZE_MAX - 2;
    nodeIndex[&graph] = GRAPH_SELF_INDEX;

    // 4. Hash each node in topological order
    hashSize(seed, nodes.size());
    for (const ShaderNode* node : nodes)
    {
        hashSize(seed, node->getImplementation().getHash());
        hashUint32(seed, node->getClassification());

        // Node inputs
        hashSize(seed, node->numInputs());
        for (const ShaderInput* input : node->getInputs())
        {
            hashPortStructure(seed, input);

            const ShaderOutput* conn = input->getConnection();
            if (conn)
            {
                auto it = nodeIndex.find(conn->getNode());
                size_t srcIdx = (it != nodeIndex.end()) ? it->second : UNKNOWN_NODE_SENTINEL;
                hashSize(seed, srcIdx);
                hashSize(seed, findOutputIndex(conn));
            }
            else
            {
                hashSize(seed, NO_CONNECTION_SENTINEL);
            }
        }

        // Node outputs
        hashSize(seed, node->numOutputs());
        for (const ShaderOutput* output : node->getOutputs())
        {
            hashPortStructure(seed, output);
        }
    }

    // 5. Hash graph output socket connections
    for (const ShaderGraphOutputSocket* socket : graph.getOutputSockets())
    {
        const ShaderOutput* conn = socket->getConnection();
        if (conn)
        {
            auto it = nodeIndex.find(conn->getNode());
            size_t srcIdx = (it != nodeIndex.end()) ? it->second : UNKNOWN_NODE_SENTINEL;
            hashSize(seed, srcIdx);
            hashSize(seed, findOutputIndex(conn));
        }
        else
        {
            hashSize(seed, NO_CONNECTION_SENTINEL);
        }
    }

    return seed;
}

MATERIALX_NAMESPACE_END
