//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader2/ShaderGraph2.h>
#include <MaterialXGenShader2/IShaderSource.h>

#include <MaterialXGenShader/ColorManagementSystem.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/Syntax.h>
#include <MaterialXGenShader/UnitSystem.h>

#include <MaterialXCore/Geom.h>
#include <MaterialXCore/Types.h>
#include <MaterialXCore/Value.h>

MATERIALX_NAMESPACE_BEGIN

// ─── populateUnitTransformMap2 ───────────────────────────────────────────────

void ShaderGraph2::populateUnitTransformMap2(UnitSystemPtr unitSystem,
                                              ShaderPort* shaderPort,
                                              DataHandle portHandle,
                                              const IShaderSource& source,
                                              const string& globalTargetUnitSpace,
                                              bool asInput)
{
    if (!unitSystem || globalTargetUnitSpace.empty())
    {
        return;
    }

    const string sourceUnitSpace = source.getPortUnit(portHandle);
    if (!shaderPort || sourceUnitSpace.empty())
    {
        return;
    }

    const string unitType = source.getPortUnitType(portHandle);
    if (!isValidHandle(source.getUnitTypeDefByName(unitType)))
    {
        return;
    }

    string targetUnitSpace = source.getPortActiveUnit(portHandle);
    if (targetUnitSpace.empty())
    {
        targetUnitSpace = globalTargetUnitSpace;
    }

    const bool supportedType = (shaderPort->getType() == Type::FLOAT   ||
                                shaderPort->getType() == Type::VECTOR2 ||
                                shaderPort->getType() == Type::VECTOR3 ||
                                shaderPort->getType() == Type::VECTOR4);
    if (supportedType)
    {
        UnitTransform transform(sourceUnitSpace, targetUnitSpace, shaderPort->getType(), unitType);
        if (unitSystem->supportsTransform(transform))
        {
            shaderPort->setUnit(sourceUnitSpace);
            if (asInput)
            {
                _inputUnitTransformMap.emplace_back(static_cast<ShaderInput*>(shaderPort), transform);
            }
            else
            {
                _outputUnitTransformMap.emplace_back(static_cast<ShaderOutput*>(shaderPort), transform);
            }
        }
    }
}

// ─── applyInputTransforms2 ───────────────────────────────────────────────────

void ShaderGraph2::applyInputTransforms2(DataHandle nodeHandle,
                                          ShaderNode* shaderNode,
                                          const IShaderSource& source,
                                          GenContext& context)
{
    ColorManagementSystemPtr cms = context.getShaderGenerator().getColorManagementSystem();
    UnitSystemPtr unitSystem     = context.getShaderGenerator().getUnitSystem();

    const string targetColorSpace = context.getOptions().targetColorSpaceOverride.empty()
                                  ? source.getActiveColorSpace()
                                  : context.getOptions().targetColorSpaceOverride;
    const string& targetDistanceUnit = context.getOptions().targetDistanceUnit;

    // Determine node-level color/multioutput classification from the NodeDef.
    DataHandle ndH = source.getNodeDef(nodeHandle);
    const string nodeDefType       = isValidHandle(ndH) ? source.getNodeDefType(ndH) : EMPTY_STRING;
    const bool nodeIsColorType     = (nodeDefType == "color3" || nodeDefType == "color4");
    const bool nodeIsMultiOutputType = (nodeDefType == "multioutput");

    const size_t inputCount = source.getNodeInputCount(nodeHandle);
    for (size_t i = 0; i < inputCount; ++i)
    {
        DataHandle inp = source.getNodeInput(nodeHandle, i);
        if (!isValidHandle(inp))
        {
            continue;
        }

        if (source.portHasValue(inp) || source.portHasInterfaceName(inp))
        {
            const string sourceColorSpace = source.getPortActiveColorSpace(inp);
            const string inputType        = source.getPortType(inp);

            if (inputType == FILENAME_TYPE_STRING && (nodeIsColorType || nodeIsMultiOutputType))
            {
                // Filename input on a color or multi-output node: transforms apply
                // to the outputs rather than the input itself.
                //
                // NOTE: The parent-chain interface color-space adjustment that
                // ShaderGraph::applyInputTransforms performs via
                // context.getParentNodes() requires a ConstNodePtr and is not
                // replicated here.  This is a known limitation for non-MX backends.
                for (ShaderOutput* shaderOutput : shaderNode->getOutputs())
                {
                    populateColorTransformMap(cms, shaderOutput,
                                             sourceColorSpace, targetColorSpace, false);
                    populateUnitTransformMap2(unitSystem, shaderOutput, inp,
                                              source, targetDistanceUnit, false);
                }
            }
            else
            {
                const string inputName     = source.getPortName(inp);
                ShaderInput*  shaderInput  = shaderNode->getInput(inputName);
                populateColorTransformMap(cms, shaderInput,
                                         sourceColorSpace, targetColorSpace, true);
                populateUnitTransformMap2(unitSystem, shaderInput, inp,
                                          source, targetDistanceUnit, true);
            }
        }
    }
}

// ─── initializeNode2 ─────────────────────────────────────────────────────────

void ShaderGraph2::initializeNode2(DataHandle nodeHandle,
                                    ShaderNode* shaderNode,
                                    ConstNodeDefPtr nodeDef,
                                    const IShaderSource& source,
                                    GenContext& context)
{
    // 1. Copy input values, paths, units, and color spaces from node-instance
    //    inputs — replicates the core of ShaderNode::initialize().
    //
    //    _impl->setValues(const Node&, ...) is intentionally NOT called here
    //    because it requires a ConstNodePtr.  The only known override is
    //    HwImageNode::setValues (UDIM UV normalization).  If that code path is
    //    needed, the caller must fall back to createNode(ConstNodePtr) via the
    //    getMxNode() bridge.
    const size_t inputCount = source.getNodeInputCount(nodeHandle);
    for (size_t i = 0; i < inputCount; ++i)
    {
        DataHandle inp = source.getNodeInput(nodeHandle, i);
        if (!isValidHandle(inp))
        {
            continue;
        }

        const string inputName = source.getPortName(inp);
        ShaderInput* shaderInput = shaderNode->getInput(inputName);
        InputPtr nodedefInput    = nodeDef->getInput(inputName);
        if (!shaderInput || !nodedefInput)
        {
            continue;
        }

        // Reconstruct the resolved value from its string representation.
        const string valueStr = source.getPortValueString(inp);
        if (!valueStr.empty())
        {
            const TypeDesc type = context.getTypeDesc(nodedefInput->getType());
            const string& enumNames = nodedefInput->getAttribute(ValueElement::ENUM_ATTRIBUTE);
            std::pair<TypeDesc, ValuePtr> enumResult;
            if (context.getShaderGenerator().getSyntax().remapEnumeration(
                    valueStr, type, enumNames, enumResult))
            {
                shaderInput->setValue(enumResult.second);
            }
            else
            {
                ValuePtr value = Value::createValueFromStrings(valueStr, nodedefInput->getType());
                shaderInput->setValue(value);
            }
        }

        // Set the element path; prefer the interface input's path when bound.
        string path = source.getPortPath(inp);
        DataHandle ifaceInp = source.getPortInterfaceInput(inp);
        if (isValidHandle(ifaceInp))
        {
            const string ifacePath = source.getPortPath(ifaceInp);
            if (!ifacePath.empty())
            {
                path = ifacePath;
            }
        }
        if (!path.empty())
        {
            shaderInput->setPath(path);
        }
    }

    // 2. Fallback paths for inputs that have no node-instance entry.
    //    Replicates the second loop in ShaderNode::initialize().
    const string nodePath = source.getElementPath(nodeHandle);
    for (const InputPtr& ndInput : nodeDef->getActiveInputs())
    {
        ShaderInput* shaderInput = shaderNode->getInput(ndInput->getName());
        if (shaderInput && shaderInput->getPath().empty())
        {
            shaderInput->setPath(nodePath + NAME_PATH_SEPARATOR + ndInput->getName());
        }
    }

    // 3. Connect inputs that are bound to a named graph interface input socket.
    //    Replicates the interface-name loop in ShaderGraph::createNode(ConstNodePtr).
    for (size_t i = 0; i < inputCount; ++i)
    {
        DataHandle inp = source.getNodeInput(nodeHandle, i);
        if (!isValidHandle(inp) || !source.portHasInterfaceName(inp))
        {
            continue;
        }

        const string ifaceName = source.getPortInterfaceName(inp);
        if (ifaceName.empty())
        {
            continue;
        }

        ShaderGraphInputSocket* inputSocket = getInputSocket(ifaceName);
        if (inputSocket)
        {
            ShaderInput* shaderInput = shaderNode->getInput(source.getPortName(inp));
            if (shaderInput)
            {
                shaderInput->makeConnection(inputSocket);
            }
        }
    }

    // 4. DefaultGeomProp: create and connect geomprop nodes for unconnected inputs.
    //    Replicates the geomprop loop in ShaderGraph::createNode(ConstNodePtr).
    //    Uses the NodeDef (a library object always present for any backend) to find
    //    the GeomPropDef — addDefaultGeomNode() looks up node defs via _document,
    //    which must be set for this to succeed.
    for (const InputPtr& ndInput : nodeDef->getActiveInputs())
    {
        ShaderInput* shaderInput = shaderNode->getInput(ndInput->getName());
        if (!shaderInput)
        {
            continue;
        }

        // Skip if the node instance has an explicit connection to this input.
        DataHandle nodeInpH = source.getNodeInputByName(nodeHandle, ndInput->getName());
        const bool hasNodeConnection = isValidHandle(nodeInpH) &&
                                       isValidHandle(source.getInputConnectedNode(nodeInpH));
        if (hasNodeConnection || shaderInput->getConnection())
        {
            continue;
        }

        GeomPropDefPtr geomProp = ndInput->getDefaultGeomProp();
        if (geomProp)
        {
            addDefaultGeomNode2(shaderInput, *geomProp, source, context);
        }
    }

    // 5. Apply color-space and unit transform nodes.
    applyInputTransforms2(nodeHandle, shaderNode, source, context);
}

// ─── addInputSocketFromPort3 (shared helper) ─────────────────────────────────

void ShaderGraph2::addInputSocketFromPort3(DataHandle portHandle,
                                            const IShaderSource& source,
                                            GenContext& context)
{
    const string portName  = source.getPortName(portHandle);
    const string portType  = source.getPortType(portHandle);
    const string valueStr  = source.getPortValueString(portHandle);
    const string enumNames = source.getPortAttribute(portHandle, ValueElement::ENUM_ATTRIBUTE);
    const TypeDesc typeDesc = context.getTypeDesc(portType);

    ShaderGraphInputSocket* inputSocket = nullptr;
    std::pair<TypeDesc, ValuePtr> enumResult;
    if (context.getShaderGenerator().getSyntax().remapEnumeration(
            valueStr, typeDesc, enumNames, enumResult))
    {
        inputSocket = addInputSocket(portName, enumResult.first);
        inputSocket->setValue(enumResult.second);
    }
    else
    {
        inputSocket = addInputSocket(portName, typeDesc);
        if (!valueStr.empty())
        {
            ValuePtr value = Value::createValueFromStrings(valueStr, portType);
            inputSocket->setValue(value);
        }
    }

    if (source.portIsUniform(portHandle))
    {
        inputSocket->setUniform();
    }

    if (source.portHasDefaultGeomProp(portHandle))
    {
        DataHandle geomPropH = source.getPortDefaultGeomProp(portHandle);
        if (isValidHandle(geomPropH))
        {
            inputSocket->setGeomProp(source.getGeomPropDefName(geomPropH));
        }
    }
}

// ─── addInputSocketsFrom*3 ────────────────────────────────────────────────────

void ShaderGraph2::addInputSocketsFromNodeDef3(DataHandle nodeDefHandle,
                                                const IShaderSource& source,
                                                GenContext& context)
{
    const size_t n = source.getNodeDefInputCount(nodeDefHandle);
    for (size_t i = 0; i < n; ++i)
    {
        DataHandle portH = source.getNodeDefInput(nodeDefHandle, i);
        if (isValidHandle(portH))
        {
            addInputSocketFromPort3(portH, source, context);
        }
    }
}

void ShaderGraph2::addInputSocketsFromNodeGraph3(DataHandle nodeGraphHandle,
                                                   const IShaderSource& source,
                                                   GenContext& context)
{
    const size_t n = source.getNodeGraphInputCount(nodeGraphHandle);
    for (size_t i = 0; i < n; ++i)
    {
        DataHandle portH = source.getNodeGraphInput(nodeGraphHandle, i);
        if (isValidHandle(portH))
        {
            addInputSocketFromPort3(portH, source, context);
        }
    }
}

void ShaderGraph2::addInputSocketsFromNode3(DataHandle nodeHandle,
                                              const IShaderSource& source,
                                              GenContext& context)
{
    const size_t n = source.getNodeInputCount(nodeHandle);
    for (size_t i = 0; i < n; ++i)
    {
        DataHandle portH = source.getNodeInput(nodeHandle, i);
        if (isValidHandle(portH))
        {
            addInputSocketFromPort3(portH, source, context);
        }
    }
}

// ─── addDefaultGeomNode2 ─────────────────────────────────────────────────────

void ShaderGraph2::addDefaultGeomNode2(ShaderInput* input, const GeomPropDef& geomprop,
                                        const IShaderSource& source, GenContext& context)
{
    const string geomNodeName = "geomprop_" + geomprop.getName();
    ShaderNode* node = getNode(geomNodeName);

    if (!node)
    {
        // Find the NodeDef for the geometric node referenced by the geomprop.
        // Replicates ShaderGraph::addDefaultGeomNode but uses IShaderSource
        // instead of _document->getNodeDef() so no document pointer is needed.
        const string geomNodeDefName = "ND_" + geomprop.getGeomProp() + "_" + input->getType().getName();
        DataHandle ndH = source.getNodeDefByName(geomNodeDefName);
        ConstNodeDefPtr geomNodeDef = source.getMxNodeDefByHandle(ndH);
        if (!geomNodeDef)
        {
            throw ExceptionShaderGenError("Could not find a nodedef named '" + geomNodeDefName +
                                          "' for defaultgeomprop on input '" + input->getFullName() + "'");
        }

        ShaderNodePtr geomNode = ShaderNode::create(this, geomNodeName, *geomNodeDef, context);
        addNode(geomNode);

        // Set node inputs if given.
        const string& namePath = geomprop.getNamePath();
        const string& space = geomprop.getSpace();
        if (!space.empty())
        {
            ShaderInput* spaceInput = geomNode->getInput(GeomPropDef::SPACE_ATTRIBUTE);
            ValueElementPtr nodeDefSpaceInput = geomNodeDef->getActiveValueElement(GeomPropDef::SPACE_ATTRIBUTE);
            if (spaceInput && nodeDefSpaceInput)
            {
                std::pair<TypeDesc, ValuePtr> enumResult;
                const string& enumNames = nodeDefSpaceInput->getAttribute(ValueElement::ENUM_ATTRIBUTE);
                const TypeDesc portType = context.getTypeDesc(nodeDefSpaceInput->getType());
                if (context.getShaderGenerator().getSyntax().remapEnumeration(space, portType, enumNames, enumResult))
                {
                    spaceInput->setValue(enumResult.second);
                }
                else
                {
                    spaceInput->setValue(Value::createValue<string>(space));
                }
                spaceInput->setPath(namePath);
            }
        }
        const string& index = geomprop.getIndex();
        if (!index.empty())
        {
            ShaderInput* indexInput = geomNode->getInput("index");
            if (indexInput)
            {
                indexInput->setValue(Value::createValue<string>(index));
                indexInput->setPath(namePath);
            }
        }
        const string& geomProp = geomprop.getGeomProp();
        if (!geomProp.empty())
        {
            ShaderInput* geomPropInput = geomNode->getInput(GeomPropDef::GEOM_PROP_ATTRIBUTE);
            if (geomPropInput)
            {
                geomPropInput->setValue(Value::createValue<string>(geomProp));
                geomPropInput->setPath(namePath);
            }
        }

        node = geomNode.get();

        // Assign a unique variable name for the node output.
        const Syntax& syntax = context.getShaderGenerator().getSyntax();
        ShaderOutput* output = node->getOutput();
        string variable = output->getFullName();
        variable = syntax.getVariableName(variable, output->getType(), _identifiers);
        output->setVariable(variable);
    }

    input->makeConnection(node->getOutput());
}

MATERIALX_NAMESPACE_END
