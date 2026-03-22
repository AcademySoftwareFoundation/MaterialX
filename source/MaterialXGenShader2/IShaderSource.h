//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GENSHADER2_ISHADERSOURCE_H
#define MATERIALX_GENSHADER2_ISHADERSOURCE_H

/// @file
/// Abstract interface for providing shader graph data to MaterialXGenShader2.
///
/// IShaderSource decouples shader graph construction from the MaterialX data
/// model.  Any system that can answer the queries below — a MaterialX Document,
/// a USD/Hydra HdMaterialNetwork, a runtime node graph, etc. — can drive shader
/// generation without ever constructing a MaterialX Element hierarchy.
///
/// ## Handle contract
///
/// Every object in the graph (nodes, inputs, outputs, NodeDefs, GeomPropDefs)
/// is represented by a DataHandle — an opaque uint64_t whose interpretation is
/// entirely up to the implementing class.  Implementations may store:
///   • a raw pointer cast to uint64_t
///   • a stable index into a flat array
///   • a string hash
///   • a packed (type, index) pair
///
/// Handles are valid for the lifetime of the IShaderSource that issued them.
/// They are not transferable across IShaderSource instances.
///
/// InvalidHandle (0) is the sentinel for "absent / not found".

#include <MaterialXGenShader2/DataHandle.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Types.h>

#include <string>
#include <vector>

MATERIALX_NAMESPACE_BEGIN

/// @class IShaderSource
/// Read-only, pull-based interface used by ShaderGraphBuilder to construct a
/// ShaderGraph without touching the MaterialX Element hierarchy directly.
///
/// All methods are const (the source is never mutated during generation).
/// Methods that can fail return InvalidHandle or an empty string rather than
/// throwing; callers must check isValidHandle() where appropriate.
class MX_GENSHADER2_API IShaderSource
{
  public:
    virtual ~IShaderSource() = default;

    // ─── Root ─────────────────────────────────────────────────────────────────

    /// Returns a handle to the root of the graph being generated.
    /// This corresponds to the ElementPtr passed to ShaderGenerator::generate()
    /// — typically an Output element or a surface/material Node.
    virtual DataHandle getRootElement() const = 0;

    // ─── Element classification ───────────────────────────────────────────────

    /// True if the handle refers to a node instance (not a NodeDef).
    virtual bool isNode(DataHandle elem) const = 0;

    /// True if the handle refers to an Output element at the top of a NodeGraph
    /// or Document (i.e. the root socket of a sub-graph).
    virtual bool isOutput(DataHandle elem) const = 0;

    /// True if the handle refers to a NodeGraph (a compound node with its own
    /// internal graph and an associated NodeDef interface).
    virtual bool isNodeGraph(DataHandle elem) const = 0;

    // ─── Element identity ─────────────────────────────────────────────────────

    /// Short name of the element (e.g. "base_color").
    virtual string getElementName(DataHandle elem) const = 0;

    /// Globally-unique dot-separated path (e.g. "SR_marble/fractal3d_1").
    /// Used as the map key when de-duplicating nodes in ShaderGraph.
    virtual string getElementPath(DataHandle elem) const = 0;

    // ─── Node topology ────────────────────────────────────────────────────────

    /// Number of active inputs on a node instance.
    virtual size_t getNodeInputCount(DataHandle node) const = 0;

    /// Returns the i-th active input handle on a node.
    virtual DataHandle getNodeInput(DataHandle node, size_t index) const = 0;

    /// Looks up an input by name on a node instance.
    /// Returns InvalidHandle if not found.
    virtual DataHandle getNodeInputByName(DataHandle node, const string& name) const = 0;

    /// Returns the upstream node connected to a given input, or InvalidHandle
    /// if the input carries a literal value (is not connected).
    virtual DataHandle getInputConnectedNode(DataHandle input) const = 0;

    /// When the upstream node has multiple outputs, this returns the name of
    /// the specific output that feeds into this input.  Returns an empty string
    /// when connected to the default (first) output.
    virtual string getInputConnectedOutputName(DataHandle input) const = 0;

    /// For an Output element at the root of a sub-graph, returns the upstream
    /// node it is connected to (or InvalidHandle if unconnected).
    virtual DataHandle getOutputConnectedNode(DataHandle output) const = 0;

    /// Returns the parent container of a node: either a NodeGraph handle or
    /// InvalidHandle if the node lives directly in the root document.
    virtual DataHandle getNodeParentGraph(DataHandle node) const = 0;

    // ─── Node definition lookup ───────────────────────────────────────────────

    /// Returns the name of the NodeDef for a node (e.g. "ND_standard_surface_surfaceshader").
    virtual string getNodeDefName(DataHandle node) const = 0;

    /// Resolves and returns the NodeDef handle for a node instance.
    /// Returns InvalidHandle if the NodeDef cannot be found.
    virtual DataHandle getNodeDef(DataHandle node) const = 0;

    /// Looks up a NodeDef by its fully-qualified name in the document/registry.
    /// Returns InvalidHandle if not found.
    virtual DataHandle getNodeDefByName(const string& nodeDefName) const = 0;

    // ─── NodeDef interface ────────────────────────────────────────────────────

    /// The output type string of the NodeDef (e.g. "surfaceshader", "color3").
    virtual string getNodeDefType(DataHandle nodeDef) const = 0;

    /// Number of inputs declared on a NodeDef.
    virtual size_t getNodeDefInputCount(DataHandle nodeDef) const = 0;

    /// Returns the i-th input handle from a NodeDef.
    virtual DataHandle getNodeDefInput(DataHandle nodeDef, size_t index) const = 0;

    /// Looks up an input by name on a NodeDef. Returns InvalidHandle if absent.
    virtual DataHandle getNodeDefInputByName(DataHandle nodeDef, const string& name) const = 0;

    /// Number of outputs declared on a NodeDef.
    virtual size_t getNodeDefOutputCount(DataHandle nodeDef) const = 0;

    /// Returns the i-th output handle from a NodeDef.
    virtual DataHandle getNodeDefOutput(DataHandle nodeDef, size_t index) const = 0;

    /// Looks up an output by name on a NodeDef. Returns InvalidHandle if absent.
    virtual DataHandle getNodeDefOutputByName(DataHandle nodeDef, const string& name) const = 0;

    /// Returns all value elements (both inputs and outputs) of a NodeDef,
    /// needed to populate ShaderNode metadata and classification.
    virtual size_t getNodeDefValueElementCount(DataHandle nodeDef) const = 0;
    virtual DataHandle getNodeDefValueElement(DataHandle nodeDef, size_t index) const = 0;

    /// True if the value element handle refers to an Output (vs. an Input).
    virtual bool valueElementIsOutput(DataHandle valueElem) const = 0;

    /// Named attribute on a NodeDef (e.g. doc, version, nodegroup).
    virtual string getNodeDefAttribute(DataHandle nodeDef, const string& attrName) const = 0;
    virtual void   getNodeDefAttributeNames(DataHandle nodeDef, StringVec& names) const = 0;

    // ─── NodeGraph interface ──────────────────────────────────────────────────

    /// Returns the NodeDef handle that defines the interface for a NodeGraph.
    virtual DataHandle getNodeGraphNodeDef(DataHandle nodeGraph) const = 0;

    /// The name of a NodeGraph element.
    virtual string getNodeGraphName(DataHandle nodeGraph) const = 0;

    /// Number of inputs declared directly on a NodeGraph element.
    /// Used when the NodeGraph has no associated NodeDef.
    virtual size_t getNodeGraphInputCount(DataHandle nodeGraph) const = 0;

    /// Returns the i-th input on a NodeGraph.
    virtual DataHandle getNodeGraphInput(DataHandle nodeGraph, size_t index) const = 0;

    /// Returns the parent NodeGraph of an Output element, or InvalidHandle if
    /// the Output lives directly in the root Document.
    virtual DataHandle getOutputParentNodeGraph(DataHandle output) const = 0;

    // ─── Port (Input / Output ValueElement) queries ───────────────────────────

    /// Short name of the port (e.g. "base_color").
    virtual string getPortName(DataHandle port) const = 0;

    /// Type string of the port (e.g. "color3", "float").
    virtual string getPortType(DataHandle port) const = 0;

    /// Dot-separated path of the port element (used for metadata / error messages).
    virtual string getPortPath(DataHandle port) const = 0;

    /// Resolved value as a string, or empty if the port is connected / has no value.
    /// This combines getResolvedValue()->getValueString() from the MX API.
    virtual string getPortValueString(DataHandle port) const = 0;

    /// True if the port has a literal value (i.e. not connected and not empty).
    virtual bool portHasValue(DataHandle port) const = 0;

    /// Retrieves a named XML attribute on a port element (e.g. "enum", "enumvalues",
    /// "uifolder").  Returns an empty string if the attribute is absent.
    virtual string getPortAttribute(DataHandle port, const string& attrName) const = 0;
    virtual void   getPortAttributeNames(DataHandle port, StringVec& names) const = 0;

    // ─── Interface binding (NodeGraph input propagation) ──────────────────────

    /// True if a node instance input is bound to a named interface input.
    virtual bool portHasInterfaceName(DataHandle port) const = 0;

    /// The interface name this input is bound to (meaningful only when
    /// portHasInterfaceName() returns true).
    virtual string getPortInterfaceName(DataHandle port) const = 0;

    /// Returns the interface input (on the enclosing NodeGraph/Document) that
    /// this port is bound to, or InvalidHandle if unbound.
    virtual DataHandle getPortInterfaceInput(DataHandle port) const = 0;

    // ─── Geometric default property ───────────────────────────────────────────

    /// True if the input has a defaultgeomprop that should drive its value
    /// when no explicit value or connection is present.
    virtual bool portHasDefaultGeomProp(DataHandle port) const = 0;

    /// Returns a handle to the GeomPropDef referred to by a port's
    /// defaultgeomprop attribute.  Returns InvalidHandle if not present.
    virtual DataHandle getPortDefaultGeomProp(DataHandle port) const = 0;

    // ─── Unit metadata ────────────────────────────────────────────────────────

    virtual string getPortUnit(DataHandle port) const = 0;
    virtual string getPortUnitType(DataHandle port) const = 0;

    /// The resolved unit after applying document/scope overrides.
    virtual string getPortActiveUnit(DataHandle port) const = 0;

    // ─── Color space metadata ─────────────────────────────────────────────────

    /// The color space declared directly on the port (may be empty).
    virtual string getPortColorSpace(DataHandle port) const = 0;

    /// The resolved color space after applying document/scope overrides.
    virtual string getPortActiveColorSpace(DataHandle port) const = 0;

    // ─── Uniformity ───────────────────────────────────────────────────────────

    /// True if the input is declared uniform (constant across the geometry).
    virtual bool portIsUniform(DataHandle port) const = 0;

    // ─── GeomPropDef queries ──────────────────────────────────────────────────

    /// The element name of the GeomPropDef (e.g. "geomprop_Nworld").
    virtual string getGeomPropDefName(DataHandle geomPropDef) const = 0;

    /// The geometric property being bound (e.g. "Nworld", "UV0").
    virtual string getGeomPropDefProp(DataHandle geomPropDef) const = 0;

    /// Dot-separated element path of the GeomPropDef.
    virtual string getGeomPropDefPath(DataHandle geomPropDef) const = 0;

    /// The coordinate space for the geometric property (e.g. "world", "object").
    virtual string getGeomPropDefSpace(DataHandle geomPropDef) const = 0;

    /// The index (for multi-set attributes like UV sets).
    virtual string getGeomPropDefIndex(DataHandle geomPropDef) const = 0;

    // ─── Document-level queries ───────────────────────────────────────────────

    /// The active (resolved) color space for the document/network.
    virtual string getActiveColorSpace() const = 0;

    /// Looks up a UnitTypeDef by its type name (e.g. "distance").
    /// Returns InvalidHandle if not found.  Used to drive unit transform nodes.
    virtual DataHandle getUnitTypeDefByName(const string& unitTypeName) const = 0;

    // ─── MX compatibility bridge ──────────────────────────────────────────────
    // These optional methods allow ShaderGraphBuilder to fall back to the
    // existing MaterialX Element API for steps not yet fully expressible through
    // the pure IShaderSource queries above.
    //
    // getMxDocument()     — no longer required by ShaderGraphBuilder.
    //                       Retained for compatibility.  Passing nullptr to the
    //                       ShaderGraph2 constructor is now safe for GLSL/MDL.
    //
    // getMxNodeDef()      — required by both build() entry points and
    //                       createConnectedNodes().  NodeDefs live in the loaded
    //                       library, so any backend that can drive generation will
    //                       have them available.
    //
    // getMxNodeDefByHandle() — required by addDefaultGeomNode2() to construct
    //                       geomprop shader nodes.  Must be implemented alongside
    //                       getNodeDefByName() for correct geomprop support.
    //
    // getMxNode()         — no longer required by ShaderGraphBuilder.
    //                       Retained for compatibility; callers that need
    //                       HwImageNode::setValues (UDIM UV normalization) still
    //                       call createNode(ConstNodePtr) manually.
    //
    // Non-MX sources may leave any of these as nullptr; ShaderGraphBuilder will
    // throw a descriptive error if a still-required method returns nullptr.

    /// Returns the underlying MaterialX Document, or nullptr for non-MX sources.
    virtual ConstDocumentPtr getMxDocument() const { return nullptr; }

    /// Returns the MaterialX NodeDef for a node handle, or nullptr for non-MX sources.
    virtual ConstNodeDefPtr getMxNodeDef(DataHandle /*node*/) const { return nullptr; }

    /// Returns the MaterialX NodeDef for a NodeDef handle (not a Node handle).
    /// Distinct from getMxNodeDef() which takes a Node handle and follows getNodeDef().
    /// Used by addDefaultGeomNode2 to construct geomprop shader nodes.
    virtual ConstNodeDefPtr getMxNodeDefByHandle(DataHandle /*nodeDefHandle*/) const { return nullptr; }

    /// Returns the MaterialX Node element for a node handle, or nullptr for non-MX sources.
    /// ShaderGraphBuilder no longer calls this; it is retained for callers that
    /// need ShaderNodeImpl::setValues (e.g. HwImageNode UDIM normalization).
    virtual ConstNodePtr getMxNode(DataHandle /*node*/) const { return nullptr; }
};

MATERIALX_NAMESPACE_END

#endif // MATERIALX_GENSHADER2_ISHADERSOURCE_H
