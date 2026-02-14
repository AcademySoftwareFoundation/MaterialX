//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGraphEditor/Layout.h>

#include <queue>
#include <unordered_set>

namespace
{

// Minimum horizontal gap between the right edge of one layer and the left
// edge of the next layer.
const float LAYER_SPACING = 110.0f;

// Minimum vertical gap between nodes in the same layer.
const float NODE_SPACING = 40.0f;

// X position of the rightmost layer (layer 0, before font-scale).
const float START_X = 1200.0f;

// Number of barycenter sweep iterations for crossing minimization.
const int CROSSING_PASSES = 4;

// Fallback node size when the editor hasn't measured a node yet.
const ImVec2 DEFAULT_NODE_SIZE = ImVec2(138, 116);

} // anonymous namespace

void Layout::clear()
{
    _nodes.clear();
    _layers.clear();
    _nextVirtualId = -1;
}

void Layout::buildGraph(const std::vector<UiNodePtr>& nodes, const std::vector<UiEdge>& edges)
{
    for (const UiNodePtr& uiNode : nodes)
    {
        // Skip group nodes.
        if (!uiNode->getMessage().empty())
        {
            continue;
        }

        int id = uiNode->getId();
        Node& n = _nodes[id];
        n.id = id;
        ImVec2 size = ed::GetNodeSize(id);
        if (size.x <= 0 || size.y <= 0)
        {
            size = DEFAULT_NODE_SIZE;
        }
        n.width = size.x;
        n.height = size.y;
    }

    for (const UiEdge& edge : edges)
    {
        if (!edge.getUp() || !edge.getDown())
        {
            continue;
        }
        int srcId = edge.getUp()->getId();
        int dstId = edge.getDown()->getId();
        if (_nodes.find(srcId) == _nodes.end() || _nodes.find(dstId) == _nodes.end())
        {
            continue;
        }
        _nodes[srcId].downstream.push_back(dstId);
        _nodes[dstId].upstream.push_back(srcId);
    }
}

LayoutResults Layout::compute(const std::vector<UiNodePtr>& nodes,
                              const std::vector<UiEdge>& edges,
                              const std::vector<int>& outputNodeIds,
                              float fontScale)
{
    clear();
    buildGraph(nodes, edges);

    if (_nodes.empty())
    {
        return {};
    }

    assignLayers(outputNodeIds);
    if (options.insertVirtualNodes)
    {
        insertVirtualNodes();
    }
    if (options.minimizeCrossings)
    {
        minimizeCrossings();
    }
    assignCoordinates(fontScale);

    // Collect results for non-virtual nodes only.
    LayoutResults results;
    for (const auto& pair : _nodes)
    {
        if (!pair.second.isVirtual)
        {
            results[pair.first] = mx::Vector2(pair.second.x, pair.second.y);
        }
    }
    return results;
}

// Phase 1: Assign layers using reverse topological order from output nodes.
// Outputs are placed at layer 0 (rightmost). Each node's layer is
// max(layer of downstream neighbors) + 1.
void Layout::assignLayers(const std::vector<int>& outputNodeIds)
{
    // Collect all output nodes: explicitly provided ones plus any with no downstream edges.
    std::unordered_set<int> outputSet(outputNodeIds.begin(), outputNodeIds.end());
    for (const auto& pair : _nodes)
    {
        if (pair.second.downstream.empty())
        {
            outputSet.insert(pair.first);
        }
    }

    // BFS from output nodes in reverse topological order.
    std::queue<int> queue;
    for (int id : outputSet)
    {
        if (_nodes.find(id) != _nodes.end())
        {
            _nodes[id].layer = 0;
            queue.push(id);
        }
    }

    while (!queue.empty())
    {
        int curr = queue.front();
        queue.pop();
        int nextLayer = _nodes[curr].layer + 1;

        for (int upId : _nodes[curr].upstream)
        {
            Node& upNode = _nodes[upId];
            if (upNode.layer < nextLayer)
            {
                upNode.layer = nextLayer;
                queue.push(upId);
            }
        }
    }

    // Assign any remaining unvisited nodes to layer 0.
    for (auto& pair : _nodes)
    {
        if (pair.second.layer < 0)
        {
            pair.second.layer = 0;
        }
    }

    // Build _layers vector.
    int maxLayer = 0;
    for (const auto& pair : _nodes)
    {
        if (pair.second.layer > maxLayer)
        {
            maxLayer = pair.second.layer;
        }
    }

    _layers.resize(maxLayer + 1);
    for (const auto& pair : _nodes)
    {
        _layers[pair.second.layer].push_back(pair.first);
    }
}

// Phase 2: Insert virtual nodes for edges that span more than one layer.
void Layout::insertVirtualNodes()
{
    // Collect long edges to process.
    std::vector<std::pair<int, int>> longEdges;
    for (const auto& pair : _nodes)
    {
        const Node& src = pair.second;
        for (int downId : src.downstream)
        {
            int span = src.layer - _nodes[downId].layer;
            if (span > 1)
            {
                longEdges.push_back({ src.id, downId });
            }
        }
    }

    // Iterate over long edges.
    for (const auto& edge : longEdges)
    {
        int srcId = edge.first;
        int dstId = edge.second;
        int srcLayer = _nodes[srcId].layer;
        int dstLayer = _nodes[dstId].layer;

        // Remove the direct edge between source and destination.
        auto& srcDown = _nodes[srcId].downstream;
        srcDown.erase(std::remove(srcDown.begin(), srcDown.end(), dstId), srcDown.end());
        auto& dstUp = _nodes[dstId].upstream;
        dstUp.erase(std::remove(dstUp.begin(), dstUp.end(), srcId), dstUp.end());

        // Create chain of virtual nodes.
        int prevId = srcId;
        for (int layer = srcLayer - 1; layer > dstLayer; --layer)
        {
            Node virtualNode;
            virtualNode.id = _nextVirtualId--;
            virtualNode.layer = layer;
            virtualNode.isVirtual = true;
            _nodes[virtualNode.id] = virtualNode;

            // Link previous node to this virtual node.
            _nodes[prevId].downstream.push_back(virtualNode.id);
            _nodes[virtualNode.id].upstream.push_back(prevId);

            _layers[layer].push_back(virtualNode.id);
            prevId = virtualNode.id;
        }

        // Link the last virtual node (or source if span==2) to the destination.
        _nodes[prevId].downstream.push_back(dstId);
        _nodes[dstId].upstream.push_back(prevId);
    }
}

// Connectivity priority: nodes connected in both directions (main chain)
// are ordered before nodes connected in only one direction (leaf nodes).
int Layout::connectivityPriority(int nodeId) const
{
    const Node& n = _nodes.at(nodeId);
    int dirs = (!n.upstream.empty() ? 1 : 0) + (!n.downstream.empty() ? 1 : 0);
    int total = static_cast<int>(n.upstream.size() + n.downstream.size());
    return dirs * 1000 + total;
}

// Set initial ordering within each layer, placing main-chain nodes
// before leaf nodes so the primary flow gets the best Y positions.
void Layout::initializeOrder()
{
    for (size_t i = 0; i < _layers.size(); ++i)
    {
        std::stable_sort(_layers[i].begin(), _layers[i].end(), [&](int a, int b)
        {
            return connectivityPriority(a) > connectivityPriority(b);
        });
        for (size_t j = 0; j < _layers[i].size(); ++j)
        {
            _nodes[_layers[i][j]].order = static_cast<int>(j);
        }
    }
}

// Compute the barycenter (average position) of a node's neighbors in the adjacent layer.
float Layout::barycenter(int nodeId, bool useDownstream) const
{
    const Node& node = _nodes.at(nodeId);
    const std::vector<int>& neighbors = useDownstream ? node.downstream : node.upstream;

    if (neighbors.empty())
    {
        return static_cast<float>(node.order);
    }

    float sum = 0.0f;
    for (int nId : neighbors)
    {
        sum += static_cast<float>(_nodes.at(nId).order);
    }
    return sum / static_cast<float>(neighbors.size());
}

// Count crossings between two adjacent layers.
int Layout::countCrossings(int layerIndex) const
{
    if (layerIndex <= 0 || layerIndex >= static_cast<int>(_layers.size()))
    {
        return 0;
    }

    const std::vector<int>& upperLayer = _layers[layerIndex];
    const std::vector<int>& lowerLayer = _layers[layerIndex - 1];

    // Build position map for the lower layer.
    std::unordered_map<int, int> lowerPos;
    for (size_t i = 0; i < lowerLayer.size(); ++i)
    {
        lowerPos[lowerLayer[i]] = static_cast<int>(i);
    }

    // Collect edges from upper layer to lower layer as (upperOrder, lowerOrder) pairs.
    std::vector<std::pair<int, int>> edgePairs;
    for (size_t i = 0; i < upperLayer.size(); ++i)
    {
        const Node& n = _nodes.at(upperLayer[i]);
        for (int downId : n.downstream)
        {
            auto it = lowerPos.find(downId);
            if (it != lowerPos.end())
            {
                edgePairs.emplace_back(static_cast<int>(i), it->second);
            }
        }
    }

    // Count inversions: a crossing occurs when edge (u1, l1) and (u2, l2)
    // have u1 < u2 but l1 > l2 (or vice versa).
    int crossings = 0;
    for (size_t i = 0; i < edgePairs.size(); ++i)
    {
        for (size_t j = i + 1; j < edgePairs.size(); ++j)
        {
            if ((edgePairs[i].first < edgePairs[j].first && edgePairs[i].second > edgePairs[j].second) ||
                (edgePairs[i].first > edgePairs[j].first && edgePairs[i].second < edgePairs[j].second))
            {
                ++crossings;
            }
        }
    }
    return crossings;
}

// Phase 3: Minimize edge crossings using barycenter heuristic.
void Layout::minimizeCrossings()
{
    initializeOrder();

    // Save best ordering.
    auto saveLayers = _layers;
    auto countAllCrossings = [&]() -> int
    {
        int total = 0;
        for (size_t i = 1; i < _layers.size(); ++i)
        {
            total += countCrossings(static_cast<int>(i));
        }
        return total;
    };

    int bestCrossings = countAllCrossings();

    for (int pass = 0; pass < CROSSING_PASSES; ++pass)
    {
        // Forward sweep: layer 1 to max, using downstream barycenter.
        // Tiebreaker: higher connectivity priority first.
        for (size_t i = 1; i < _layers.size(); ++i)
        {
            std::vector<int>& layer = _layers[i];
            std::stable_sort(layer.begin(), layer.end(), [&](int a, int b)
            {
                float ba = barycenter(a, true);
                float bb = barycenter(b, true);
                if (ba != bb) return ba < bb;
                return connectivityPriority(a) > connectivityPriority(b);
            });
            for (size_t j = 0; j < layer.size(); ++j)
            {
                _nodes[layer[j]].order = static_cast<int>(j);
            }
        }

        // Backward sweep: max-1 to 0, using upstream barycenter.
        // Tiebreaker: higher connectivity priority first.
        for (int i = static_cast<int>(_layers.size()) - 2; i >= 0; --i)
        {
            std::vector<int>& layer = _layers[i];
            std::stable_sort(layer.begin(), layer.end(), [&](int a, int b)
            {
                float ba = barycenter(a, false);
                float bb = barycenter(b, false);
                if (ba != bb) return ba < bb;
                return connectivityPriority(a) > connectivityPriority(b);
            });
            for (size_t j = 0; j < layer.size(); ++j)
            {
                _nodes[layer[j]].order = static_cast<int>(j);
            }
        }

        int currentCrossings = countAllCrossings();
        if (currentCrossings < bestCrossings)
        {
            bestCrossings = currentCrossings;
            saveLayers = _layers;
        }
    }

    // Restore best ordering.
    _layers = saveLayers;
    for (size_t i = 0; i < _layers.size(); ++i)
    {
        for (size_t j = 0; j < _layers[i].size(); ++j)
        {
            _nodes[_layers[i][j]].order = static_cast<int>(j);
        }
    }
}

// Refine Y positions for a single layer by shifting nodes toward the median
// Y of their neighbors, resolving overlaps, and centering the layer.
void Layout::refineLayerY(int layerIndex, bool preferDownstream)
{
    std::vector<int>& layer = _layers[layerIndex];

    // Compute ideal Y for each node and track desired center.
    float idealCenterSum = 0.0f;
    int idealCount = 0;
    for (int nodeId : layer)
    {
        Node& n = _nodes[nodeId];
        const std::vector<int>& preferred = preferDownstream ? n.downstream : n.upstream;
        const std::vector<int>& fallback = preferDownstream ? n.upstream : n.downstream;
        const std::vector<int>& neighbors = !preferred.empty() ? preferred : fallback;
        if (neighbors.empty())
        {
            continue;
        }

        std::vector<float> neighborYs;
        for (int nId : neighbors)
        {
            const Node& nn = _nodes[nId];
            neighborYs.push_back(nn.y + nn.height * 0.5f);
        }
        std::sort(neighborYs.begin(), neighborYs.end());
        float medianY = neighborYs[neighborYs.size() / 2];
        n.y = medianY - n.height * 0.5f;
        idealCenterSum += medianY;
        idealCount++;
    }

    // Resolve overlaps while maintaining order.
    for (size_t j = 1; j < layer.size(); ++j)
    {
        Node& prev = _nodes[layer[j - 1]];
        Node& curr = _nodes[layer[j]];
        float minY = prev.y + prev.height + NODE_SPACING;
        if (curr.y < minY)
        {
            curr.y = minY;
        }
    }

    // Center the layer around the desired position.
    if (idealCount > 0 && layer.size() > 1)
    {
        float idealCenter = idealCenterSum / idealCount;
        float actualCenterSum = 0.0f;
        for (int nodeId : layer)
        {
            actualCenterSum += _nodes[nodeId].y + _nodes[nodeId].height * 0.5f;
        }
        float actualCenter = actualCenterSum / static_cast<float>(layer.size());
        float shift = idealCenter - actualCenter;
        for (int nodeId : layer)
        {
            _nodes[nodeId].y += shift;
        }
    }
}

// Phase 4: Assign X and Y coordinates.
void Layout::assignCoordinates(float fontScale)
{
    // Compute max node width per layer for variable-width spacing.
    std::vector<float> maxWidthPerLayer(_layers.size(), 0.0f);
    for (size_t i = 0; i < _layers.size(); ++i)
    {
        for (int nodeId : _layers[i])
        {
            const Node& n = _nodes[nodeId];
            if (n.width > maxWidthPerLayer[i])
            {
                maxWidthPerLayer[i] = n.width;
            }
        }
    }

    // X positions: layer 0 is rightmost, accumulate leftward using
    // per-layer widths so wide nodes don't overlap adjacent layers.
    std::vector<float> layerX(_layers.size(), 0.0f);
    layerX[0] = START_X * fontScale;
    for (size_t i = 1; i < _layers.size(); ++i)
    {
        layerX[i] = layerX[i - 1] - (maxWidthPerLayer[i] + LAYER_SPACING) * fontScale;
    }
    for (auto& pair : _nodes)
    {
        Node& n = pair.second;
        n.x = layerX[n.layer];
    }

    // Initial Y positions: stack nodes in each layer with spacing.
    for (size_t i = 0; i < _layers.size(); ++i)
    {
        float y = 0.0f;
        for (int nodeId : _layers[i])
        {
            Node& n = _nodes[nodeId];
            n.y = y;
            y += n.height + NODE_SPACING;
        }
    }

    // Y refinement passes: shift nodes toward the median Y of their neighbors.
    if (options.refinePositions)
    {
        for (int pass = 0; pass < 2; ++pass)
        {
            // Forward pass: align toward downstream neighbors.
            for (size_t i = 1; i < _layers.size(); ++i)
            {
                refineLayerY(static_cast<int>(i), true);
            }

            // Backward pass: align toward upstream neighbors.
            for (int i = static_cast<int>(_layers.size()) - 2; i >= 0; --i)
            {
                refineLayerY(i, false);
            }
        }
    }
}
