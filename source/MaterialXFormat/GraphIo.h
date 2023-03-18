//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GRAPHIO_H
#define MATERIALX_GRAPHIO_H

/// @file
/// Support for the MaterialX GraphElement interchange classes

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/Export.h>
#include <MaterialXFormat/File.h>

MATERIALX_NAMESPACE_BEGIN

class GraphIo;
class DotGraphIo;
class MermaidGraphIo;

/// A shared pointer to a GraphIo
using GraphIoPtr = shared_ptr<GraphIo>;
/// A shared pointer to a const GraphIo
using ConstGraphIoPtr = shared_ptr<const GraphIo>;

/// A shared pointer to a DotGraphIo
using DotGraphIoPtr = shared_ptr<DotGraphIo>;
/// A shared pointer to a const GraphIo
using ConstDotGraphIoPtr = shared_ptr<const DotGraphIo>;

/// A shared pointer to a MermaidGraphIo
using MermaidGraphIoPtr = shared_ptr<MermaidGraphIo>;
/// A shared pointer to a const MermaidGraphIo
using ConstMermaidGraphIoPtr = shared_ptr<const MermaidGraphIo>;

/// @class NodeIO
///     Node information extracted during graph traversal and 
///     provided to utility writer methods. This includes user
///     interface information hints such as UI label and shape.
/// 
class MX_FORMAT_API NodeIO
{
  public:
    /// UI node shapes
    enum class NodeShape
    {
        BOX = 0,        /// Box shape. Used for non interface nodes
        ROUNDEDBOX = 1, /// Rounded box shape. Used to indicate interface input and output nodes
        DIAMOND = 1,    /// Diamond shape. Used to indicate conditionals.
    };

    /// Uniique Node identifier. This identifier is unique per MaterialX Document.
    string identifier;

    /// Node UI label for the identifier
    string uilabel;

    /// MaterialX node category string
    string category;

    /// MaterialX node group string
    string group;

    /// Node UI shape. Default is box.
    NodeShape uishape = NodeShape::BOX;
};

/// @class GraphIoGenOptions
///     Generation options for GraphIo generators    
/// 
class MX_FORMAT_API GraphIoGenOptions
{
  public:
    GraphIoGenOptions() {}
    virtual ~GraphIoGenOptions() {}

    /// Option on whether to emit the graph header 
    void setWriteGraphHeader(bool val)
    {
        _writeGraphHeader = val;
    }

    /// Get whether to emit the graph header. This is enabled by default.
    bool getWriteGraphHeader() const
    {
        return _writeGraphHeader;
    }

    /// Option on whether to write node labels using the category of the node as the label as
    /// opposed to the unique name of the Element. Default is to write category names.
    void setWriteCategories(bool val)
    {
        _writeCategories = val;
    }

    /// Get whether to write categories for node labels
    bool getWriteCategories() const
    {
        return _writeCategories;
    }

    /// Option on whether to write subgraph groupings or not. 
    /// For a node definition, this can be turned off as definition graphs
    /// are in general single nodegraphs. By default subgraphs are written.
    void setWriteSubgraphs(bool val)
    {
        _writeSubgraphs = val;
    }

    /// Get whether option to write subgraphs is enabled
    bool getWriteSubgraphs() const
    {
        return _writeSubgraphs;
    }

    /// Graph writing orientation hint. Some formats may or
    /// may not support one or more of these options.
    enum class Orientation
    {
        TOP_DOWN,
        BOTTOM_UP,
        LEFT_RIGHT,
        RIGHT_LEFT
    };

    /// Hint on the orientation to write the graph.  
    /// The default orientation is set to TOP_DOWN which means the root is
    /// at the top and upstream downs are positioned downward from the root.
    /// @param val Orientationn hint
    void setOrientation(Orientation val)
    {
        _orientation = val;
    }

    /// Get whether option to write subgraphs is enabled.
    /// @return Orientation hint
    Orientation getOrientation() const
    {
        return _orientation;
    }

  protected:
    /// Write greaph head
    bool _writeGraphHeader = true;

    /// Write category labels
    bool _writeCategories = true;

    /// Write subgraphs
    bool _writeSubgraphs = true;

    /// Orientation hint
    Orientation _orientation = Orientation::TOP_DOWN;
};

/// @class GraphIo
/// <summary>
///     Interface defining classes which interpret a GraphElement and 
///     generate output in the desired output format. 
/// 
///     The output is assumed to be representable by a string, but subclasses
///     may choose their own runtime representation.
/// 
///     The class indicates which formats are supported via a list of string identifiers.
///     These identifiers can be used to register the class with a GraphIoRegistry.
/// 
///     The default traversal logic will call into a set of utities
///     which are responsible for emitting the appropriate output in the desired format.
///     A derived class may chose to implement their own traversal logic as well.
///     
/// </summary>
class MX_FORMAT_API GraphIo
{
  public:
    GraphIo(){};
    virtual ~GraphIo(){};

    /// Returns a list of formats that the GraphIo can convert from to
    const StringSet& supportsFormats() const
    {
        return _formats;
    }

    /// @name I/O methods
    /// @{

    /// Traverse a graph and return a string withe formatted output
    /// Derived classes must implement this method.
    /// @param graph GraphElement to write
    /// @param roots Optional list of roots to GraphIo what upstream elements to consider>
    /// @returns String result
    virtual string write(GraphElementPtr graph, const StringVec& roots) = 0;

    /// @}
    /// @name Options to set before writing
    /// @{

    /// Get options for generation
    const GraphIoGenOptions& getGenOptions() const
    {
        return _genOptions;
    }

    /// Set options for generation
    void setGenOptions(const GraphIoGenOptions& options)
    {
        _genOptions = options;
    }

  protected:
    /// @name Generation Utilties
    /// @{

    /// Traverse a graph and call into utility methods to emit which handle emitting the 
    /// appropriate output. 
    /// @param graph GraphElement to traverse
    /// @param roots Optional list of roots indicating what upstream elements to consider.
    ///              If empty, then all of the graph's outputs are traversed.
    virtual void emitGraph(GraphElementPtr graph, const StringVec& roots);
      
    /// Emit the root node only. Called when there are no downstream connections.
    /// @param root Root node
    virtual void emitRootNode(const NodeIO& root)
    {
        (void)(root);
    }

    /// Emit the upstream node on a connection
    /// @param node Upstream node
    virtual void emitUpstreamNode(const NodeIO& node)
    {
        (void)(node);
    }

    /// Emit the connection from an upstream node to a downstream node. 
    /// @param outputName Name of the upstream output
    /// @param outputLabel ui labelfor the upstream output
    /// @param inputName name of input on downstream node 
    /// @param channelName name of channel on downstream input. 
    /// This would be the equivalent of having an `extract` node between upstream output and downstream input.
    /// This argument may be empty.
    virtual void emitConnection(
        const string& outputName,
        const string& outputLabel, 
        const string& inputName,
        const string& channelName) 
    {
        (void)(outputName);
        (void)(outputLabel);
        (void)(inputName);
        (void)(channelName);
    }

    /// Emit a connection between an interface input and a input on a node in a GraphElement
    /// @param interfaceId Identifier for interface
    /// @param interfaceInputName Identifier for interface input
    /// @param inputName Name of input on interior node
    /// @param interiorNode Interior node information
    virtual void emitInterfaceConnection(
        const string& interfaceId,
        const string& interfaceInputName,
        const string& inputName,
        const NodeIO& interiorNode)
    {
        (void)(interfaceId);
        (void)(interfaceInputName);
        (void)(inputName);
        (void)(interiorNode);
    }

    /// Emit downstream node and label
    /// @param node Node information to write
    /// @param inputLabel input on node
    virtual void emitDownstreamNode(const NodeIO& node,
                                    const string& inputLabel)
    {
        (void)(node);
        (void)(inputLabel);
    }

    /// Emit sub-graph groupings. For now the groupings supported are NodeGraphs
    /// @param  subGraphs List of sub-graphs to write
    virtual void emitSubgraphs(std::unordered_map<string, StringSet> subGraphs)
    {
        (void)(subGraphs);
    }

    /// Write GraphElement
    virtual void emitGraphString()
    {
    }

    /// @}
    /// @name Subgraph Handling Utilities
    /// @{

    /// Add an Element label subgraph list. 
    /// Given a node and label, the label will to be used to add an identifier to the subgraph list.
    /// @param subGraphs Structure to maintain a list of unique node identifiers per subgraph name. The subgraph name is a unique Document name.
    /// @param node The parent of this node if it exists is the subgraph to add the label to
    /// @param label The string used to create a uniquely labelled element in the subgraph. The subgraph path will be prepended to the lable
    /// @return Derived label name
    /// </summary>
    string addNodeToSubgraph(std::unordered_map<string, StringSet>& subGraphs, 
                             const ElementPtr node, 
                             const string& label) const;

    /// @}

    /// Storage for formats supported
    StringSet _formats;

    /// Map containing restricted keywords and their replacement for identifiers
    StringMap _restrictedMap;

    /// Write options
    GraphIoGenOptions _genOptions;

    /// Written output
    string _graphResult;
};

/// @class DotGraphIo
///     Class which provides support for outputting to GraphViz dot format.
/// 
class MX_FORMAT_API DotGraphIo : public GraphIo
{
public:
    DotGraphIo()
    {
        // Dot files
        _formats.insert("dot");
    }
    virtual ~DotGraphIo() = default;

    static DotGraphIoPtr create();

    string write(GraphElementPtr graph, const StringVec& roots) override;

  protected:
    void emitRootNode(const NodeIO& root) override;
    void emitUpstreamNode(const NodeIO& node) override;
    void emitConnection(
        const string& outputName,
        const string& outputLabel,
        const string& inputName,
        const string& channelName) override;
    void emitInterfaceConnection(
        const string& interfaceId,
        const string& interfaceInputName,
        const string& inputName,
        const NodeIO& interiorNode) override;
    void emitDownstreamNode(const NodeIO& node, const string& inputName) override;
    void emitSubgraphs(
        std::unordered_map<string, StringSet> subGraphs) override;
    void emitGraphString() override;
};
    

/// @class MermaidGraphIo
/// 
///     Class which provides support for outputting to Mermaid format.
///     Note that the output only includes the Mermaid graph itself and does not include 
///     wrappers for embedding into Markdown or HTML.
/// 
class MX_FORMAT_API MermaidGraphIo : public GraphIo
{
  public:
    MermaidGraphIo()
    {
        // Markdown and Markdown diagrams
        _formats.insert("md");
        _formats.insert("mmd");

        // Add restricted keywords
        _restrictedMap["default"] = "dfault";
    }
    virtual ~MermaidGraphIo() = default;

    /// Creator
    static MermaidGraphIoPtr create();

    string write(GraphElementPtr graph, const StringVec& roots) override;

  protected:
    void emitRootNode(const NodeIO& root) override;
    void emitUpstreamNode(const NodeIO& node) override;
    void emitConnection(
        const string& outputName,
        const string& outputLabel,
        const string& inputName,
        const string& channelName) override;
    void emitInterfaceConnection(
        const string& interfaceId,
        const string& interfaceInputName,
        const string& inputName,
        const NodeIO& interiorNode) override;
    void emitDownstreamNode(const NodeIO& node, const string& inputName) override;
    void emitSubgraphs(
        std::unordered_map<string, StringSet> subGraphs) override;
    void emitGraphString() override;
};

MATERIALX_NAMESPACE_END

#endif
