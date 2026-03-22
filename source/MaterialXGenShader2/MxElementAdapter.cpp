//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader2/MxElementAdapter.h>

#include <MaterialXCore/Node.h>
#include <MaterialXCore/Interface.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Geom.h>
#include <MaterialXCore/Unit.h>

MATERIALX_NAMESPACE_BEGIN

// ─── Construction ─────────────────────────────────────────────────────────────

MxElementAdapter::MxElementAdapter(ConstDocumentPtr document, ConstElementPtr root)
    : _document(document)
    , _root(root)
{
}

// ─── Root ─────────────────────────────────────────────────────────────────────

DataHandle MxElementAdapter::getRootElement() const
{
    return toHandle(_root.get());
}

// ─── Element classification ───────────────────────────────────────────────────

bool MxElementAdapter::isNode(DataHandle elem) const
{
    const Element* e = toElement(elem);
    return e && e->isA<Node>();
}

bool MxElementAdapter::isOutput(DataHandle elem) const
{
    const Element* e = toElement(elem);
    return e && e->isA<Output>();
}

bool MxElementAdapter::isNodeGraph(DataHandle elem) const
{
    const Element* e = toElement(elem);
    return e && e->isA<NodeGraph>();
}

// ─── Element identity ─────────────────────────────────────────────────────────

string MxElementAdapter::getElementName(DataHandle elem) const
{
    const Element* e = toElement(elem);
    return e ? e->getName() : EMPTY_STRING;
}

string MxElementAdapter::getElementPath(DataHandle elem) const
{
    const Element* e = toElement(elem);
    return e ? e->getNamePath() : EMPTY_STRING;
}

// ─── Node topology ────────────────────────────────────────────────────────────

size_t MxElementAdapter::getNodeInputCount(DataHandle node) const
{
    const Element* e = toElement(node);
    if (!e) return 0;
    ConstNodePtr n = e->asA<Node>();
    return n ? n->getActiveInputs().size() : 0;
}

DataHandle MxElementAdapter::getNodeInput(DataHandle node, size_t index) const
{
    const Element* e = toElement(node);
    if (!e) return InvalidHandle;
    ConstNodePtr n = e->asA<Node>();
    if (!n) return InvalidHandle;
    vector<InputPtr> inputs = n->getActiveInputs();
    if (index >= inputs.size()) return InvalidHandle;
    return toHandle(inputs[index].get());
}

DataHandle MxElementAdapter::getNodeInputByName(DataHandle node, const string& name) const
{
    const Element* e = toElement(node);
    if (!e) return InvalidHandle;
    ConstNodePtr n = e->asA<Node>();
    if (!n) return InvalidHandle;
    return toHandle(n->getInput(name).get());
}

DataHandle MxElementAdapter::getInputConnectedNode(DataHandle input) const
{
    const Element* e = toElement(input);
    if (!e) return InvalidHandle;
    ConstInputPtr inp = e->asA<Input>();
    if (!inp) return InvalidHandle;
    return toHandle(inp->getConnectedNode().get());
}

string MxElementAdapter::getInputConnectedOutputName(DataHandle input) const
{
    const Element* e = toElement(input);
    if (!e) return EMPTY_STRING;
    shared_ptr<const PortElement> pe = e->asA<PortElement>();
    if (!pe) return EMPTY_STRING;
    // getOutputString() returns the name of the specific output on the upstream
    // node that feeds this input (e.g. "outr"/"outg"/"outb" for multi-output nodes).
    // Returns empty string when connected to the default output.
    return pe->getOutputString();
}

DataHandle MxElementAdapter::getOutputConnectedNode(DataHandle output) const
{
    const Element* e = toElement(output);
    if (!e) return InvalidHandle;
    ConstOutputPtr out = e->asA<Output>();
    if (!out) return InvalidHandle;
    return toHandle(out->getConnectedNode().get());
}

DataHandle MxElementAdapter::getNodeParentGraph(DataHandle node) const
{
    const Element* e = toElement(node);
    if (!e) return InvalidHandle;
    ConstElementPtr parent = e->getParent();
    if (!parent || !parent->isA<NodeGraph>()) return InvalidHandle;
    return toHandle(parent.get());
}

// ─── Node definition lookup ───────────────────────────────────────────────────

string MxElementAdapter::getNodeDefName(DataHandle node) const
{
    const Element* e = toElement(node);
    if (!e) return EMPTY_STRING;
    ConstNodePtr n = e->asA<Node>();
    if (!n) return EMPTY_STRING;
    ConstNodeDefPtr nd = n->getNodeDef();
    return nd ? nd->getName() : EMPTY_STRING;
}

DataHandle MxElementAdapter::getNodeDef(DataHandle node) const
{
    const Element* e = toElement(node);
    if (!e) return InvalidHandle;
    ConstNodePtr n = e->asA<Node>();
    if (!n) return InvalidHandle;
    return toHandle(n->getNodeDef().get());
}

DataHandle MxElementAdapter::getNodeDefByName(const string& nodeDefName) const
{
    return toHandle(_document->getNodeDef(nodeDefName).get());
}

// ─── NodeDef interface ────────────────────────────────────────────────────────

string MxElementAdapter::getNodeDefType(DataHandle nodeDef) const
{
    const Element* e = toElement(nodeDef);
    if (!e) return EMPTY_STRING;
    ConstNodeDefPtr nd = e->asA<NodeDef>();
    if (!nd) return EMPTY_STRING;
    return nd->getType();
}

size_t MxElementAdapter::getNodeDefInputCount(DataHandle nodeDef) const
{
    const Element* e = toElement(nodeDef);
    if (!e) return 0;
    ConstNodeDefPtr nd = e->asA<NodeDef>();
    if (!nd) return 0;
    return nd->getActiveInputs().size();
}

DataHandle MxElementAdapter::getNodeDefInput(DataHandle nodeDef, size_t index) const
{
    const Element* e = toElement(nodeDef);
    if (!e) return InvalidHandle;
    ConstNodeDefPtr nd = e->asA<NodeDef>();
    if (!nd) return InvalidHandle;
    vector<InputPtr> inputs = nd->getActiveInputs();
    if (index >= inputs.size()) return InvalidHandle;
    return toHandle(inputs[index].get());
}

DataHandle MxElementAdapter::getNodeDefInputByName(DataHandle nodeDef, const string& name) const
{
    const Element* e = toElement(nodeDef);
    if (!e) return InvalidHandle;
    ConstNodeDefPtr nd = e->asA<NodeDef>();
    if (!nd) return InvalidHandle;
    return toHandle(nd->getInput(name).get());
}

size_t MxElementAdapter::getNodeDefOutputCount(DataHandle nodeDef) const
{
    const Element* e = toElement(nodeDef);
    if (!e) return 0;
    ConstNodeDefPtr nd = e->asA<NodeDef>();
    if (!nd) return 0;
    return nd->getActiveOutputs().size();
}

DataHandle MxElementAdapter::getNodeDefOutput(DataHandle nodeDef, size_t index) const
{
    const Element* e = toElement(nodeDef);
    if (!e) return InvalidHandle;
    ConstNodeDefPtr nd = e->asA<NodeDef>();
    if (!nd) return InvalidHandle;
    vector<OutputPtr> outputs = nd->getActiveOutputs();
    if (index >= outputs.size()) return InvalidHandle;
    return toHandle(outputs[index].get());
}

DataHandle MxElementAdapter::getNodeDefOutputByName(DataHandle nodeDef, const string& name) const
{
    const Element* e = toElement(nodeDef);
    if (!e) return InvalidHandle;
    ConstNodeDefPtr nd = e->asA<NodeDef>();
    if (!nd) return InvalidHandle;
    return toHandle(nd->getOutput(name).get());
}

size_t MxElementAdapter::getNodeDefValueElementCount(DataHandle nodeDef) const
{
    const Element* e = toElement(nodeDef);
    if (!e) return 0;
    ConstNodeDefPtr nd = e->asA<NodeDef>();
    if (!nd) return 0;
    return nd->getActiveValueElements().size();
}

DataHandle MxElementAdapter::getNodeDefValueElement(DataHandle nodeDef, size_t index) const
{
    const Element* e = toElement(nodeDef);
    if (!e) return InvalidHandle;
    ConstNodeDefPtr nd = e->asA<NodeDef>();
    if (!nd) return InvalidHandle;
    vector<ValueElementPtr> elems = nd->getActiveValueElements();
    if (index >= elems.size()) return InvalidHandle;
    return toHandle(elems[index].get());
}

bool MxElementAdapter::valueElementIsOutput(DataHandle valueElem) const
{
    const Element* e = toElement(valueElem);
    return e && e->isA<Output>();
}

string MxElementAdapter::getNodeDefAttribute(DataHandle nodeDef, const string& attrName) const
{
    const Element* e = toElement(nodeDef);
    if (!e) return EMPTY_STRING;
    return e->getAttribute(attrName);
}

void MxElementAdapter::getNodeDefAttributeNames(DataHandle nodeDef, StringVec& names) const
{
    const Element* e = toElement(nodeDef);
    if (!e) return;
    names = e->getAttributeNames();
}

// ─── NodeGraph interface ──────────────────────────────────────────────────────

DataHandle MxElementAdapter::getNodeGraphNodeDef(DataHandle nodeGraph) const
{
    const Element* e = toElement(nodeGraph);
    if (!e) return InvalidHandle;
    ConstNodeGraphPtr ng = e->asA<NodeGraph>();
    if (!ng) return InvalidHandle;
    return toHandle(ng->getNodeDef().get());
}

string MxElementAdapter::getNodeGraphName(DataHandle nodeGraph) const
{
    const Element* e = toElement(nodeGraph);
    if (!e) return EMPTY_STRING;
    ConstNodeGraphPtr ng = e->asA<NodeGraph>();
    return ng ? ng->getName() : EMPTY_STRING;
}

size_t MxElementAdapter::getNodeGraphInputCount(DataHandle nodeGraph) const
{
    const Element* e = toElement(nodeGraph);
    if (!e) return 0;
    ConstNodeGraphPtr ng = e->asA<NodeGraph>();
    if (!ng) return 0;
    return ng->getActiveInputs().size();
}

DataHandle MxElementAdapter::getNodeGraphInput(DataHandle nodeGraph, size_t index) const
{
    const Element* e = toElement(nodeGraph);
    if (!e) return InvalidHandle;
    ConstNodeGraphPtr ng = e->asA<NodeGraph>();
    if (!ng) return InvalidHandle;
    vector<InputPtr> inputs = ng->getActiveInputs();
    if (index >= inputs.size()) return InvalidHandle;
    return toHandle(inputs[index].get());
}

DataHandle MxElementAdapter::getOutputParentNodeGraph(DataHandle output) const
{
    const Element* e = toElement(output);
    if (!e) return InvalidHandle;
    ConstElementPtr parent = e->getParent();
    if (!parent || !parent->isA<NodeGraph>()) return InvalidHandle;
    return toHandle(parent.get());
}

// ─── Port queries ─────────────────────────────────────────────────────────────

string MxElementAdapter::getPortName(DataHandle port) const
{
    const Element* e = toElement(port);
    return e ? e->getName() : EMPTY_STRING;
}

string MxElementAdapter::getPortType(DataHandle port) const
{
    const Element* e = toElement(port);
    if (!e) return EMPTY_STRING;
    ConstValueElementPtr ve = e->asA<ValueElement>();
    return ve ? ve->getType() : EMPTY_STRING;
}

string MxElementAdapter::getPortPath(DataHandle port) const
{
    const Element* e = toElement(port);
    return e ? e->getNamePath() : EMPTY_STRING;
}

string MxElementAdapter::getPortValueString(DataHandle port) const
{
    const Element* e = toElement(port);
    if (!e) return EMPTY_STRING;
    ConstValueElementPtr ve = e->asA<ValueElement>();
    if (!ve) return EMPTY_STRING;
    ValuePtr val = ve->getResolvedValue();
    return val ? val->getValueString() : EMPTY_STRING;
}

bool MxElementAdapter::portHasValue(DataHandle port) const
{
    const Element* e = toElement(port);
    if (!e) return false;
    ConstValueElementPtr ve = e->asA<ValueElement>();
    return ve && ve->hasValue();
}

string MxElementAdapter::getPortAttribute(DataHandle port, const string& attrName) const
{
    const Element* e = toElement(port);
    return e ? e->getAttribute(attrName) : EMPTY_STRING;
}

void MxElementAdapter::getPortAttributeNames(DataHandle port, StringVec& names) const
{
    const Element* e = toElement(port);
    if (!e) return;
    names = e->getAttributeNames();
}

// ─── Interface binding ────────────────────────────────────────────────────────

bool MxElementAdapter::portHasInterfaceName(DataHandle port) const
{
    const Element* e = toElement(port);
    if (!e) return false;
    ConstInputPtr inp = e->asA<Input>();
    return inp && inp->hasInterfaceName();
}

string MxElementAdapter::getPortInterfaceName(DataHandle port) const
{
    const Element* e = toElement(port);
    if (!e) return EMPTY_STRING;
    ConstInputPtr inp = e->asA<Input>();
    return inp ? inp->getInterfaceName() : EMPTY_STRING;
}

DataHandle MxElementAdapter::getPortInterfaceInput(DataHandle port) const
{
    const Element* e = toElement(port);
    if (!e) return InvalidHandle;
    ConstInputPtr inp = e->asA<Input>();
    if (!inp) return InvalidHandle;
    return toHandle(inp->getInterfaceInput().get());
}

// ─── Geometric default property ───────────────────────────────────────────────

bool MxElementAdapter::portHasDefaultGeomProp(DataHandle port) const
{
    const Element* e = toElement(port);
    if (!e) return false;
    ConstInputPtr inp = e->asA<Input>();
    return inp && inp->hasDefaultGeomPropString();
}

DataHandle MxElementAdapter::getPortDefaultGeomProp(DataHandle port) const
{
    const Element* e = toElement(port);
    if (!e) return InvalidHandle;
    ConstInputPtr inp = e->asA<Input>();
    if (!inp) return InvalidHandle;
    return toHandle(inp->getDefaultGeomProp().get());
}

// ─── Unit metadata ────────────────────────────────────────────────────────────

string MxElementAdapter::getPortUnit(DataHandle port) const
{
    const Element* e = toElement(port);
    if (!e) return EMPTY_STRING;
    ConstInputPtr inp = e->asA<Input>();
    return inp ? inp->getUnit() : EMPTY_STRING;
}

string MxElementAdapter::getPortUnitType(DataHandle port) const
{
    const Element* e = toElement(port);
    if (!e) return EMPTY_STRING;
    ConstInputPtr inp = e->asA<Input>();
    return inp ? inp->getUnitType() : EMPTY_STRING;
}

string MxElementAdapter::getPortActiveUnit(DataHandle port) const
{
    const Element* e = toElement(port);
    if (!e) return EMPTY_STRING;
    ConstInputPtr inp = e->asA<Input>();
    return inp ? inp->getActiveUnit() : EMPTY_STRING;
}

// ─── Color space metadata ─────────────────────────────────────────────────────

string MxElementAdapter::getPortColorSpace(DataHandle port) const
{
    const Element* e = toElement(port);
    if (!e) return EMPTY_STRING;
    ConstInputPtr inp = e->asA<Input>();
    return inp ? inp->getColorSpace() : EMPTY_STRING;
}

string MxElementAdapter::getPortActiveColorSpace(DataHandle port) const
{
    const Element* e = toElement(port);
    if (!e) return EMPTY_STRING;
    ConstInputPtr inp = e->asA<Input>();
    return inp ? inp->getActiveColorSpace() : EMPTY_STRING;
}

// ─── Uniformity ───────────────────────────────────────────────────────────────

bool MxElementAdapter::portIsUniform(DataHandle port) const
{
    const Element* e = toElement(port);
    if (!e) return false;
    ConstInputPtr inp = e->asA<Input>();
    return inp && inp->getIsUniform();
}

// ─── GeomPropDef queries ──────────────────────────────────────────────────────

string MxElementAdapter::getGeomPropDefName(DataHandle geomPropDef) const
{
    const Element* e = toElement(geomPropDef);
    return e ? e->getName() : EMPTY_STRING;
}

string MxElementAdapter::getGeomPropDefProp(DataHandle geomPropDef) const
{
    const Element* e = toElement(geomPropDef);
    if (!e) return EMPTY_STRING;
    ConstGeomPropDefPtr gp = e->asA<GeomPropDef>();
    return gp ? gp->getGeomProp() : EMPTY_STRING;
}

string MxElementAdapter::getGeomPropDefPath(DataHandle geomPropDef) const
{
    const Element* e = toElement(geomPropDef);
    return e ? e->getNamePath() : EMPTY_STRING;
}

string MxElementAdapter::getGeomPropDefSpace(DataHandle geomPropDef) const
{
    const Element* e = toElement(geomPropDef);
    if (!e) return EMPTY_STRING;
    ConstGeomPropDefPtr gp = e->asA<GeomPropDef>();
    return gp ? gp->getSpace() : EMPTY_STRING;
}

string MxElementAdapter::getGeomPropDefIndex(DataHandle geomPropDef) const
{
    const Element* e = toElement(geomPropDef);
    if (!e) return EMPTY_STRING;
    ConstGeomPropDefPtr gp = e->asA<GeomPropDef>();
    return gp ? gp->getIndex() : EMPTY_STRING;
}

// ─── Document-level queries ───────────────────────────────────────────────────

ConstNodeDefPtr MxElementAdapter::getMxNodeDef(DataHandle node) const
{
    const Element* e = toElement(node);
    if (!e) return nullptr;
    ConstNodePtr n = e->asA<Node>();
    return n ? n->getNodeDef() : nullptr;
}

ConstNodeDefPtr MxElementAdapter::getMxNodeDefByHandle(DataHandle nodeDefHandle) const
{
    const Element* e = toElement(nodeDefHandle);
    return e ? e->asA<NodeDef>() : nullptr;
}

ConstNodePtr MxElementAdapter::getMxNode(DataHandle node) const
{
    const Element* e = toElement(node);
    return e ? e->asA<Node>() : nullptr;
}

string MxElementAdapter::getActiveColorSpace() const
{
    return _document ? _document->getActiveColorSpace() : EMPTY_STRING;
}

DataHandle MxElementAdapter::getUnitTypeDefByName(const string& unitTypeName) const
{
    if (!_document) return InvalidHandle;
    return toHandle(_document->getUnitTypeDef(unitTypeName).get());
}

MATERIALX_NAMESPACE_END
