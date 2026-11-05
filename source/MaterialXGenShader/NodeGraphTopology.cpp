//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/NodeGraphTopology.h>
#include <MaterialXGenShader/Exception.h>

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Library.h>

#include <unordered_set>

MATERIALX_NAMESPACE_BEGIN

namespace
{
// PBR nodes with a weight parameter -- when weight=0, the node can be pruned.
// TODO: Could this allowlist be replaced by checking output type == BSDF and the
// presence of a topological weight input (see isTopologicalInput)?
const std::unordered_set<string> kWeightedPbrNodes = {
    "oren_nayar_diffuse_bsdf",
    "compensating_oren_nayar_diffuse_bsdf",
    "burley_diffuse_bsdf",
    "conductor_bsdf",
    "subsurface_bsdf",
    "translucent_bsdf",
    "dielectric_bsdf",
    "generalized_schlick_bsdf",
    "sheen_bsdf",
    "dielectric_tf_bsdf",
    "generalized_schlick_tf_82_bsdf",
    "sheen_zeltner_bsdf",
};

} // anonymous namespace

NodeGraphTopology::NodeGraphTopology(const NodeGraph& nodeGraph)
{
    NodeDefPtr nodeDef = nodeGraph.getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Can't find nodedef for nodegraph '" + nodeGraph.getName() + "'");
    }

    // Build per-node info (downstream ref counts and upstream dependencies)
    buildNodeInfos(nodeGraph);

    // Scan all nodes in the NodeGraph for optimization opportunities
    for (const NodePtr& node : nodeGraph.getNodes())
    {
        const string& category = node->getCategory();

        if (category == "mix")
        {
            // Mix nodes: mix input at 0 or 1 can eliminate a branch
            InputPtr mixInput = node->getActiveInput("mix");
            if (mixInput && isTopologicalInput(mixInput, nodeDef))
            {
                const string& interfaceName = mixInput->getInterfaceName();
                if (_topologicalInputs.find(interfaceName) == _topologicalInputs.end())
                {
                    _topologicalInputs.emplace(interfaceName, TopologicalInput(interfaceName, node, mixInput));
                }
            }
        }
        else if (category == "multiply")
        {
            // Multiply nodes: any float input at 0 zeroes the output
            for (const InputPtr& input : node->getActiveInputs())
            {
                if (input && isTopologicalInput(input, nodeDef))
                {
                    const string& interfaceName = input->getInterfaceName();
                    if (_topologicalInputs.find(interfaceName) == _topologicalInputs.end())
                    {
                        _topologicalInputs.emplace(interfaceName, TopologicalInput(interfaceName, node, input));
                    }
                }
            }
        }
        else if (kWeightedPbrNodes.count(category))
        {
            // PBR nodes: weight at 0 makes the node output dark/transparent
            InputPtr weightInput = node->getActiveInput("weight");
            if (weightInput && isTopologicalInput(weightInput, nodeDef))
            {
                const string& interfaceName = weightInput->getInterfaceName();
                if (_topologicalInputs.find(interfaceName) == _topologicalInputs.end())
                {
                    _topologicalInputs.emplace(interfaceName, TopologicalInput(interfaceName, node, weightInput));
                }
            }
        }
    }
}

bool NodeGraphTopology::isTopologicalInput(const InputPtr& input, const NodeDefPtr& nodeDef)
{
    // Must be connected to the NodeGraph interface
    if (!input->hasInterfaceName())
    {
        return false;
    }

    // Must be float type
    if (input->getType() != "float")
    {
        return false;
    }

    // Get the corresponding NodeDef input
    const string& interfaceName = input->getInterfaceName();
    InputPtr ndInput = nodeDef->getActiveInput(interfaceName);
    if (!ndInput)
    {
        return false;
    }
    // Check for uimin=0, uimax=1 (indicates a 0-1 weight parameter)
    if (!ndInput->hasAttribute("uimin") || !ndInput->hasAttribute("uimax"))
    {
        return false;
    }

    try
    {
        float minVal = std::stof(ndInput->getAttribute("uimin"));
        float maxVal = std::stof(ndInput->getAttribute("uimax"));
        return (minVal == 0.0f && maxVal == 1.0f);
    }
    catch (...)
    {
        return false;
    }
}

TopologicalInput::TopologicalInput(
    const string& inputName_,
    const NodePtr& node,
    const InputPtr& input) :
    inputName(inputName_)
{
    const string& category = node->getCategory();

    if (category == "mix")
    {
        // For mix nodes:
        // - When mix=0, the "fg" (foreground) branch loses a consumer
        // - When mix=1, the "bg" (background) branch loses a consumer
        // The mix node itself stays alive; we just decrement the unused input's upstream ref count
        InputPtr bgInput = node->getActiveInput("bg");
        InputPtr fgInput = node->getActiveInput("fg");

        if (fgInput && fgInput->hasNodeName())
        {
            // mix=0 means fg branch loses this consumer
            potentiallyPrunableAtValue[0].insert(fgInput->getNodeName());
        }

        if (bgInput && bgInput->hasNodeName())
        {
            // mix=1 means bg branch loses this consumer
            potentiallyPrunableAtValue[1].insert(bgInput->getNodeName());
        }
    }
    else if (category == "multiply")
    {
        // For multiply nodes with input=0:
        // The multiply node stays alive (outputs 0), but the other inputs lose a consumer
        for (const InputPtr& otherInput : node->getActiveInputs())
        {
            if (otherInput != input && otherInput->hasNodeName())
            {
                potentiallyPrunableAtValue[0].insert(otherInput->getNodeName());
            }
        }
    }
    else if (kWeightedPbrNodes.count(category))
    {
        // For PBR nodes with weight=0:
        // The PBR node itself is unconditionally pruned (replaced with dark/transparent)
        // Its upstream dependencies will be handled via ref count propagation
        prunableAtValue[0].insert(node->getName());
    }
}

void NodeGraphTopology::buildNodeInfos(const NodeGraph& nodeGraph)
{
    for (const NodePtr& node : nodeGraph.getNodes())
    {
        const string& nodeName = node->getName();
        NodeInfo& nodeInfo = _nodeInfos[nodeName];

        for (const InputPtr& input : node->getActiveInputs())
        {
            if (input->hasNodeName())
            {
                nodeInfo.upstreams.insert(input->getNodeName());
            }
        }

        // Increment downstream ref counts once per unique upstream
        for (const string& upstreamName : nodeInfo.upstreams)
        {
            _nodeInfos[upstreamName].downstreamRefCount++;
        }
    }

    // NodeGraph outputs are roots -- they also count as downstream consumers
    for (const OutputPtr& output : nodeGraph.getOutputs())
    {
        if (output->hasNodeName())
        {
            _nodeInfos[output->getNodeName()].downstreamRefCount++;
        }
    }
}

std::unique_ptr<NodeGraphPermutation> NodeGraphTopology::createPermutation(const Node& node) const
{
    if (_topologicalInputs.empty())
    {
        return nullptr;
    }

    string permutationKey;
    std::unordered_set<std::string> nodesToPrune;
    bool hasOptimization = false;

    // Working copy of ref counts for this permutation
    std::unordered_map<string, size_t> downstreamRefCounts;
    downstreamRefCounts.reserve(_nodeInfos.size());
    for (const auto& [name, nodeInfo] : _nodeInfos)
    {
        downstreamRefCounts[name] = nodeInfo.downstreamRefCount;
    }

    std::vector<string> worklist;

    // Mark a node as pruned and enqueue for upstream propagation.
    auto pruneNode = [&nodesToPrune, &worklist](const string& nodeName)
    {
        if (nodesToPrune.insert(nodeName).second)
        {
            worklist.push_back(nodeName);
        }
    };

    // A downstream consumer of `nodeName` has been eliminated. Decrement ref count
    // and prune `nodeName` if no consumers remain.
    auto removeDownstream = [&downstreamRefCounts, &pruneNode](const string& nodeName)
    {
        auto itDownstreamRefCount = downstreamRefCounts.find(nodeName);
        if (itDownstreamRefCount != downstreamRefCounts.end()
            && itDownstreamRefCount->second > 0)
        {
            itDownstreamRefCount->second--;
            if (itDownstreamRefCount->second == 0)
            {
                pruneNode(nodeName);
            }
        }
    };

    // First pass: build the key and collect initial dead nodes
    for (const auto& [inputName, topoInput] : _topologicalInputs)
    {
        char flag = 'x';  // 'x' = not optimized (connected or intermediate value)

        auto applyConstantValue = [&pruneNode, &removeDownstream](
            const std::unordered_set<std::string>& prunable,
            const std::unordered_set<std::string>& potentiallyPrunable)
        {
            for (const string& nodeName : prunable)
            {
                pruneNode(nodeName);
            }
            for (const string& nodeName : potentiallyPrunable)
            {
                removeDownstream(nodeName);
            }
        };

        // Apply the effects of a topological input being constant (0 or 1).
        // Takes topoInput by parameter (not capture) because capturing
        // structured bindings requires C++20.
        auto applyConstantInput = [&applyConstantValue, &flag](
            const TopologicalInput& topo, Input& input)
        {
            if (!input.hasValue())
            {
                return;
            }

            const float value = input.getValue()->asA<float>();
            if (value == 0.0f)
            {
                flag = '0';
                applyConstantValue(topo.prunableAtValue[0], topo.potentiallyPrunableAtValue[0]);
            }
            else if (value == 1.0f)
            {
                flag = '1';
                applyConstantValue(topo.prunableAtValue[1], topo.potentiallyPrunableAtValue[1]);
            }
        };

        // Check if this input is connected on the node instance
        if (InputPtr nodeInput = node.getInput(inputName))
        {
            // If connected to another node, can't optimize
            if (!( nodeInput->hasNodeName()
                || nodeInput->hasOutputString()
                || nodeInput->hasInterfaceName()))
            {
                applyConstantInput(topoInput, *nodeInput);
            }
        }
        else if (NodeDefPtr nodeDef = node.getNodeDef()) // Input not set on node instance - check NodeDef default value
        {
            if (InputPtr defaultInput = nodeDef->getActiveInput(inputName))
            {
                applyConstantInput(topoInput, *defaultInput);
            }
        }

        if (!permutationKey.empty())
        {
            permutationKey += ",";
        }
        permutationKey += inputName + "=" + flag;
        if (flag != 'x')
        {
            hasOptimization = true;
        }
    }

    // Worklist-driven DCE: when a node is pruned, decrement ref counts of
    // its upstream dependencies. Prune any upstream whose count hits 0.
    while (!worklist.empty())
    {
        string nodeName = worklist.back();
        worklist.pop_back();

        auto itNodeInfo = _nodeInfos.find(nodeName);
        if (itNodeInfo != _nodeInfos.end())
        {
            for (const string& upstream : itNodeInfo->second.upstreams)
            {
                removeDownstream(upstream);
            }
        }
    }

    if (!hasOptimization)
    {
        return nullptr;
    }

    return std::make_unique<NodeGraphPermutation>(
        std::move(permutationKey), std::move(nodesToPrune));
}

//
// NodeGraphTopologyCache implementation
//

NodeGraphTopologyCache::~NodeGraphTopologyCache() = default;

NodeGraphTopologyCache::NodeGraphTopologyCache(const NodeGraphTopologyCache& other)
{
    std::lock_guard<std::mutex> lock(other._cacheMutex);
    _cache = other._cache;
}

NodeGraphTopologyCache& NodeGraphTopologyCache::operator=(const NodeGraphTopologyCache& other)
{
    if (this != &other)
    {
        std::scoped_lock lock(_cacheMutex, other._cacheMutex);
        _cache = other._cache;
    }
    return *this;
}

void NodeGraphTopologyCache::clear()
{
    std::lock_guard<std::mutex> lock(_cacheMutex);
    _cache.clear();
}

std::unique_ptr<NodeGraphPermutation> NodeGraphTopologyCache::createPermutation(
    const NodeGraph& nodeGraph, const Node& node)
{
    const NodeGraphTopology& topology = getTopology(nodeGraph);
    return topology.createPermutation(node);
}

const NodeGraphTopology& NodeGraphTopologyCache::getTopology(const NodeGraph& nodeGraph)
{
    const string& ngName = nodeGraph.getName();

    // Check cache first (with lock)
    {
        std::lock_guard<std::mutex> lock(_cacheMutex);
        auto it = _cache.find(ngName);
        if (it != _cache.end())
        {
            return *it->second;
        }
    }

    // Cache miss - construct outside lock to allow parallel construction
    // of different topologies. Safe because emplace() won't overwrite.
    auto topology = std::make_shared<NodeGraphTopology>(nodeGraph);

    std::lock_guard<std::mutex> lock(_cacheMutex);
    auto [it, inserted] = _cache.emplace(ngName, std::move(topology));
    return *it->second;
}

MATERIALX_NAMESPACE_END
