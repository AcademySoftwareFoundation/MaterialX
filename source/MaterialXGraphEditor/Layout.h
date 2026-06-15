//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_LAYOUT_H
#define MATERIALX_LAYOUT_H

#include <MaterialXGraphEditor/UiNode.h>

// Map from node ID to computed position.
using LayoutResults = std::unordered_map<int, mx::Vector2>;

// A simple layout engine that implements Sugiyama-style layered graph drawing
// https://en.wikipedia.org/wiki/Layered_graph_drawing
class Layout
{
  public:
    // Options for enabling or disabling individual algorithm phases,
    // allowing developers to understand their impact on the layout.
    struct Options
    {
        bool insertVirtualNodes = true;
        bool minimizeCrossings = true;
        bool refinePositions = true;
    };

    // Compute layout positions for the given nodes and edges.
    LayoutResults compute(const std::vector<UiNodePtr>& nodes,
                          const std::vector<UiEdge>& edges,
                          const std::vector<int>& outputNodeIds,
                          float fontScale = 1.0f);

  public:
    Options options;

  private:
    struct Node
    {
        int id = 0;
        float width = 0.0f;
        float height = 0.0f;
        int layer = -1;
        int order = 0;
        float x = 0.0f;
        float y = 0.0f;
        bool isVirtual = false;
        std::vector<int> upstream;
        std::vector<int> downstream;
    };

    // Build internal graph representation from UI nodes and edges.
    void buildGraph(const std::vector<UiNodePtr>& nodes, const std::vector<UiEdge>& edges);

    // Phase 1: Assign layers using reverse topological order from output nodes.
    void assignLayers(const std::vector<int>& outputNodeIds);

    // Phase 2: Insert virtual nodes for edges that span more than one layer.
    void insertVirtualNodes();

    // Phase 3: Minimize edge crossings using barycenter heuristic.
    void minimizeCrossings();

    // Set initial ordering within each layer, placing main-chain nodes
    // before leaf nodes so the primary flow gets the best Y positions.
    void initializeOrder();

    // Sort a single layer by barycenter position and assign order values.
    void sortByBarycenter(std::vector<int>& layer, bool useDownstream);

    // Count crossings between two adjacent layers.
    int countCrossings(int layerIndex) const;

    // Compute the barycenter (average position) of a node's neighbors
    // in the adjacent layer.
    float barycenter(int nodeId, bool useDownstream) const;

    // Return a priority score favoring nodes connected in both directions
    // (main chain) over those connected in only one direction (leaf nodes).
    int connectivityPriority(int nodeId) const;

    // Phase 4: Assign X and Y coordinates.
    void assignCoordinates(float fontScale);

    // Refine Y positions for a single layer by shifting nodes toward the
    // median Y of their neighbors, resolving overlaps, and centering.
    void refineLayerY(int layerIndex, bool preferDownstream);

  private:
    std::unordered_map<int, Node> _nodes;
    std::vector<std::vector<int>> _layers;
    int _nextVirtualId = -1;
};

#endif
