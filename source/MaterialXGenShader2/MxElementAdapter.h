//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GENSHADER2_MXELEMENTADAPTER_H
#define MATERIALX_GENSHADER2_MXELEMENTADAPTER_H

/// @file
/// IShaderSource implementation backed by a MaterialX Document and root Element.
///
/// MxElementAdapter is the bridge between the existing MaterialX data model and
/// the new IShaderSource abstraction.  It wraps a (ConstDocumentPtr, ConstElementPtr)
/// pair and answers every IShaderSource query by delegating to the corresponding
/// MaterialX API call.
///
/// Handles are raw MaterialX Element* pointers cast to uint64_t.  The document
/// must remain alive for the lifetime of this adapter.

#include <MaterialXGenShader2/IShaderSource.h>

#include <MaterialXCore/Document.h>

MATERIALX_NAMESPACE_BEGIN

/// @class MxElementAdapter
/// Implements IShaderSource over a live MaterialX document + root element.
///
/// Typical usage:
/// @code
///   auto adapter = std::make_unique<MxElementAdapter>(doc, outputElement);
///   GenContextCreate ctx(glslGenerator, std::move(adapter));
///   ShaderGraph2Ptr graph = ctx.buildGraph("myShader");
/// @endcode
class MX_GENSHADER2_API MxElementAdapter : public IShaderSource
{
  public:
    /// Constructs an adapter for the given document and root output/node element.
    /// @param document  The owning document; must outlive this adapter.
    /// @param root      The root Output or Node element from which to build the graph.
    MxElementAdapter(ConstDocumentPtr document, ConstElementPtr root);

    ~MxElementAdapter() override = default;

    // ─── Root ─────────────────────────────────────────────────────────────────
    DataHandle getRootElement() const override;

    // ─── Element classification ───────────────────────────────────────────────
    bool isNode(DataHandle elem) const override;
    bool isOutput(DataHandle elem) const override;
    bool isNodeGraph(DataHandle elem) const override;

    // ─── Element identity ─────────────────────────────────────────────────────
    string getElementName(DataHandle elem) const override;
    string getElementPath(DataHandle elem) const override;

    // ─── Node topology ────────────────────────────────────────────────────────
    size_t   getNodeInputCount(DataHandle node) const override;
    DataHandle getNodeInput(DataHandle node, size_t index) const override;
    DataHandle getNodeInputByName(DataHandle node, const string& name) const override;
    DataHandle getInputConnectedNode(DataHandle input) const override;
    string   getInputConnectedOutputName(DataHandle input) const override;
    DataHandle getOutputConnectedNode(DataHandle output) const override;
    DataHandle getNodeParentGraph(DataHandle node) const override;

    // ─── Node definition lookup ───────────────────────────────────────────────
    string   getNodeDefName(DataHandle node) const override;
    DataHandle getNodeDef(DataHandle node) const override;
    DataHandle getNodeDefByName(const string& nodeDefName) const override;

    // ─── NodeDef interface ────────────────────────────────────────────────────
    string   getNodeDefType(DataHandle nodeDef) const override;
    size_t   getNodeDefInputCount(DataHandle nodeDef) const override;
    DataHandle getNodeDefInput(DataHandle nodeDef, size_t index) const override;
    DataHandle getNodeDefInputByName(DataHandle nodeDef, const string& name) const override;
    size_t   getNodeDefOutputCount(DataHandle nodeDef) const override;
    DataHandle getNodeDefOutput(DataHandle nodeDef, size_t index) const override;
    DataHandle getNodeDefOutputByName(DataHandle nodeDef, const string& name) const override;
    size_t   getNodeDefValueElementCount(DataHandle nodeDef) const override;
    DataHandle getNodeDefValueElement(DataHandle nodeDef, size_t index) const override;
    bool     valueElementIsOutput(DataHandle valueElem) const override;
    string   getNodeDefAttribute(DataHandle nodeDef, const string& attrName) const override;
    void     getNodeDefAttributeNames(DataHandle nodeDef, StringVec& names) const override;

    // ─── NodeGraph interface ──────────────────────────────────────────────────
    DataHandle getNodeGraphNodeDef(DataHandle nodeGraph) const override;
    string   getNodeGraphName(DataHandle nodeGraph) const override;
    size_t   getNodeGraphInputCount(DataHandle nodeGraph) const override;
    DataHandle getNodeGraphInput(DataHandle nodeGraph, size_t index) const override;
    DataHandle getOutputParentNodeGraph(DataHandle output) const override;

    // ─── Port queries ─────────────────────────────────────────────────────────
    string   getPortName(DataHandle port) const override;
    string   getPortType(DataHandle port) const override;
    string   getPortPath(DataHandle port) const override;
    string   getPortValueString(DataHandle port) const override;
    bool     portHasValue(DataHandle port) const override;
    string   getPortAttribute(DataHandle port, const string& attrName) const override;
    void     getPortAttributeNames(DataHandle port, StringVec& names) const override;

    // ─── Interface binding ────────────────────────────────────────────────────
    bool     portHasInterfaceName(DataHandle port) const override;
    string   getPortInterfaceName(DataHandle port) const override;
    DataHandle getPortInterfaceInput(DataHandle port) const override;

    // ─── Geometric default property ───────────────────────────────────────────
    bool     portHasDefaultGeomProp(DataHandle port) const override;
    DataHandle getPortDefaultGeomProp(DataHandle port) const override;

    // ─── Unit metadata ────────────────────────────────────────────────────────
    string   getPortUnit(DataHandle port) const override;
    string   getPortUnitType(DataHandle port) const override;
    string   getPortActiveUnit(DataHandle port) const override;

    // ─── Color space metadata ─────────────────────────────────────────────────
    string   getPortColorSpace(DataHandle port) const override;
    string   getPortActiveColorSpace(DataHandle port) const override;

    // ─── Uniformity ───────────────────────────────────────────────────────────
    bool portIsUniform(DataHandle port) const override;

    // ─── GeomPropDef queries ──────────────────────────────────────────────────
    string getGeomPropDefName(DataHandle geomPropDef) const override;
    string getGeomPropDefProp(DataHandle geomPropDef) const override;
    string getGeomPropDefPath(DataHandle geomPropDef) const override;
    string getGeomPropDefSpace(DataHandle geomPropDef) const override;
    string getGeomPropDefIndex(DataHandle geomPropDef) const override;

    // ─── Document-level queries ───────────────────────────────────────────────
    string   getActiveColorSpace() const override;
    DataHandle getUnitTypeDefByName(const string& unitTypeName) const override;

    // ─── Phase 2 MX bridge ────────────────────────────────────────────────────
    ConstDocumentPtr getMxDocument() const override { return _document; }
    ConstNodeDefPtr  getMxNodeDef(DataHandle node) const override;
    ConstNodeDefPtr  getMxNodeDefByHandle(DataHandle nodeDefHandle) const override;
    ConstNodePtr     getMxNode(DataHandle node) const override;

  private:
    /// Converts a DataHandle back to a const Element pointer.
    /// All handles issued by this adapter are Element* cast to uint64_t.
    static const Element* toElement(DataHandle h)
    {
        return reinterpret_cast<const Element*>(static_cast<uintptr_t>(h));
    }

    /// Wraps a non-null Element pointer as a handle; null becomes InvalidHandle.
    static DataHandle toHandle(const Element* e)
    {
        return e ? static_cast<DataHandle>(reinterpret_cast<uintptr_t>(e)) : InvalidHandle;
    }

    ConstDocumentPtr _document;
    ConstElementPtr  _root;
};

MATERIALX_NAMESPACE_END

#endif // MATERIALX_GENSHADER2_MXELEMENTADAPTER_H
