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

    void buildGraph(const std::vector<UiNodePtr>& nodes, const std::vector<UiEdge>& edges);
    void assignLayers(const std::vector<int>& outputNodeIds);
    void insertVirtualNodes();
    void initializeOrder();
    void minimizeCrossings();
    void assignCoordinates(float fontScale);
    void refineLayerY(int layerIndex, bool preferDownstream);
    int countCrossings(int layerIndex) const;
    float barycenter(int nodeId, bool useDownstream) const;
    int connectivityPriority(int nodeId) const;
    void clear();

  private:
    std::unordered_map<int, Node> _nodes;
    std::vector<std::vector<int>> _layers;
    int _nextVirtualId = -1;
};

#endif
