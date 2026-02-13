//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_LAYOUT_H
#define MATERIALX_LAYOUT_H

#include <MaterialXGraphEditor/UiNode.h>

// A simple layout engine that implements Sugiyama-style layered graph drawing
// https://en.wikipedia.org/wiki/Layered_graph_drawing
class Layout
{
  public:
    struct Result
    {
        float x;
        float y;
    };

    std::unordered_map<int, Result> compute(
        const std::vector<UiNodePtr>& nodes,
        const std::vector<UiEdge>& edges,
        const std::vector<int>& outputNodeIds,
        float fontScale = 1.0f);

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
        bool isDummy = false;
        std::vector<int> upstream;
        std::vector<int> downstream;
    };

    void buildGraph(const std::vector<UiNodePtr>& nodes, const std::vector<UiEdge>& edges);
    void assignLayers(const std::vector<int>& outputNodeIds);
    void insertDummyNodes();
    void initializeOrder();
    void minimizeCrossings();
    void assignCoordinates(float fontScale);
    void refineLayerY(int layerIndex, bool preferDownstream);
    int countCrossings(int layerIndex) const;
    float barycenter(int nodeId, bool useDownstream) const;
    int connectivityPriority(int nodeId) const;
    void clear();

    std::unordered_map<int, Node> _nodes;
    std::vector<std::vector<int>> _layers;
    int _nextDummyId = -1;
};

#endif
