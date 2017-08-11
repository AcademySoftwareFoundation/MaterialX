#include <NodeTranslator.h>
#include <SceneTranslator.h>
#include <Plugin.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

namespace
{
    // TODO: Use a better float compare operator
    inline bool isDifferent(float a, float b)
    {
        return fabs(a - b) >= std::numeric_limits<float>::epsilon();
    }

    string getPortMxName(const string& name, mx::ImplementationPtr impl)
    {
        if (impl)
        {
            mx::InputPtr input = impl->getInput(name);
            if (input)
            {
                const string& implname = input->getAttribute("implname");
                if (implname != mx::EMPTY_STRING)
                {
                    return implname;
                }
            }
        }
        return name;
    }
}


set<string> NodeTranslator::_attributeIgnoreList;

NodeTranslator::NodeTranslator()
    : _translatorData(nullptr)
{
}

void NodeTranslator::initialize(const MObject& mayaNode, mx::ConstDocumentPtr data)
{
    _translatorData = make_shared<TranslatorData>(mayaNode, data);
}

mx::NodeDefPtr NodeTranslator::exportNodeDef(const MObject& mayaNode, const std::string& outputType, TranslatorContext& context)
{
    const std::string nodeDefName = _translatorData->mxNodeDef + "_" + outputType;

    // Search the current document for a nodedef for this node
    mx::NodeDefPtr nodeDef = context.doc->getNodeDef(nodeDefName);
    if (nodeDef)
    {
        return nodeDef;
    }

    // Search the nodedefs document for a nodedef for this node
    nodeDef = context.nodeDefs->getNodeDef(nodeDefName);
    if (nodeDef)
    {
        return nodeDef;
    }

    //
    // Create the nodedef
    //

    MFnDependencyNode fnNode(mayaNode);

    nodeDef = context.nodeDefs->addNodeDef(nodeDefName, outputType/*_translatorData->mxDataType*/, _translatorData->mxNodeType);

    for (const Attribute& attr : _translatorData->attributes)
    {
        if (!shouldExport(getMayaName(attr.name)))
        {
            continue;
        }

        switch (attr.portType)
        {
            case INPUT_PORT:
            {
                mx::InputPtr port = nodeDef->addInput(attr.name, attr.type);
                port->setValueString(attr.value);
                break;
            }
            case PARAMETER_PORT:
            {
                mx::ParameterPtr port = nodeDef->addParameter(attr.name, attr.type);
                port->setValueString(attr.value);
                break;
            }
            case OUTPUT_PORT:
            {
                mx::OutputPtr port = nodeDef->addChild<mx::Output>(attr.name);
                port->setType(attr.type);
                break;
            }
        }
    }

    return nodeDef;
}

mx::NodePtr NodeTranslator::exportNode(const MObject& mayaNode, const std::string& outputType, mx::NodeGraphPtr parent, TranslatorContext& context)
{
    const std::string name = getNodeName(mayaNode, outputType);

    // Check if this node instance exists already
    mx::NodePtr node = parent->getNode(name);
    if (node)
    {
        return node;
    }

    mx::NodeDefPtr nodeDef = exportNodeDef(mayaNode, outputType, context);
    if (!nodeDef)
    {
        MGlobal::displayWarning(("Can't find a nodedef for node " + name + ".").c_str());
        return nullptr;
    }

    node = parent->addNode(nodeDef->getNode(), name, nodeDef->getType());

    MFnDependencyNode fnNode(mayaNode);

    // Export inputs
    for (mx::InputPtr input : nodeDef->getInputs())
    {
        const string mayaName = getMayaName(input->getName());
        MPlug plug = fnNode.findPlug(mayaName.c_str(), false);
        if (shouldExport(plug, input->getValue()))
        {
            mx::InputPtr nodeInput = node->getInput(input->getName());
            if (!nodeInput)
            {
                nodeInput = node->addInput(input->getName(), input->getType());
            }

            bool exportByValue = true;
            if (plug.isDestination())
            {
                MObject srcNodeObj = plug.source().node();
                NodeTranslatorPtr translator = Plugin::instance().getTranslator(srcNodeObj);

                if (translator && !translator->exportByValue())
                {
                    mx::NodePtr srcNode = translator->exportNode(srcNodeObj, nodeInput->getType(), parent, context);
                    if (srcNode)
                    {
                        nodeInput->setConnectedNode(srcNode);
                        exportByValue = false;
                    }
                }
            }
            if (exportByValue)
            {
                SceneTranslator::exportValue(plug, nodeInput);
            }
        }
    }

    // Export parameters
    for (mx::ParameterPtr param : nodeDef->getParameters())
    {
        const string mayaName = getMayaName(param->getName());
        MPlug plug = fnNode.findPlug(mayaName.c_str(), false);
        if (shouldExport(plug, param->getValue()))
        {
            mx::ParameterPtr nodeParam = node->getParameter(param->getName());
            if (!nodeParam)
            {
                nodeParam = node->addParameter(param->getName(), param->getType());
            }
            SceneTranslator::exportValue(plug, nodeParam);
        }
    }

    return node;
}

bool NodeTranslator::exportByValue() const
{
    return _translatorData->exportByValue;
}

bool NodeTranslator::shouldExport(const string& mayaAttrName) const
{
    return _attributeIgnoreList.find(mayaAttrName) == _attributeIgnoreList.end();
}

bool NodeTranslator::shouldExport(const MPlug& mayaPlug, mx::ValuePtr defaultValue) const
{
    // Null plugs can't be exported
    if (mayaPlug.isNull())
    {
        return false;
    }

    // Check attribute ignore list
    if (!shouldExport(mayaPlug.name().asChar()))
    {
        return false;
    }

    // Connections should always be exported
    if (mayaPlug.isDestination())
    {
        return true;
    }

    const std::string mxType = SceneTranslator::getMxType(mayaPlug.attribute());

    // Export only if value is different from default value
    if (mxType == "boolean")
    {
        return defaultValue->asA<bool>() != mayaPlug.asBool();
    }
    else if (mxType == "integer")
    {
        return defaultValue->asA<int>() != mayaPlug.asInt();
    }
    else if (mxType == "float")
    {
        return isDifferent(defaultValue->asA<float>(), mayaPlug.asFloat());
    }
    else if (mxType == "vector2")
    {
        mx::Vector2 vec = defaultValue->asA<mx::Vector2>();
        return isDifferent(vec[0], mayaPlug.child(0).asFloat()) ||
            isDifferent(vec[1], mayaPlug.child(1).asFloat());
    }
    else if (mxType == "color3")
    {
        mx::Color3 col = defaultValue->asA<mx::Color3>();
        return isDifferent(col[0], mayaPlug.child(0).asFloat()) ||
            isDifferent(col[1], mayaPlug.child(1).asFloat()) ||
            isDifferent(col[2], mayaPlug.child(2).asFloat());
    }
    else if (mxType == "vector3")
    {
        mx::Vector3 vec = defaultValue->asA<mx::Vector3>();
        return isDifferent(vec[0], mayaPlug.child(0).asFloat()) ||
            isDifferent(vec[1], mayaPlug.child(1).asFloat()) ||
            isDifferent(vec[2], mayaPlug.child(2).asFloat());
    }
    else if (mxType == "color4")
    {
        mx::Color4 col = defaultValue->asA<mx::Color4>();
        return isDifferent(col[0], mayaPlug.child(0).asFloat()) ||
            isDifferent(col[1], mayaPlug.child(1).asFloat()) ||
            isDifferent(col[2], mayaPlug.child(2).asFloat()) ||
            isDifferent(col[3], mayaPlug.child(3).asFloat());
    }
    else if (mxType == "vector4")
    {
        mx::Vector4 vec = defaultValue->asA<mx::Vector4>();
        return isDifferent(vec[0], mayaPlug.child(0).asFloat()) ||
            isDifferent(vec[1], mayaPlug.child(1).asFloat()) ||
            isDifferent(vec[2], mayaPlug.child(2).asFloat()) ||
            isDifferent(vec[3], mayaPlug.child(3).asFloat());
    }
    else if (mxType == "string" || mxType == "filename")
    {
        return defaultValue->getValueString() != mayaPlug.asString().asChar();
    }

    return false;
}

string NodeTranslator::getNodeName(const MObject& mayaNode, const std::string& outputType)
{
    static const string delim = "_";
    MFnDependencyNode fnNode(mayaNode);
    return fnNode.name().asChar() + delim + outputType;
}

NodeTranslator::TranslatorData::TranslatorData(const MObject& mayaNode, mx::ConstDocumentPtr data)
    : exportByValue(false)
{
    MFnDependencyNode fnNode(mayaNode);
    mayaNodeType = fnNode.typeName().asChar();

    // Check if data is set for this node
    mx::NodeDefPtr nodeDef = data->getNodeDef(mayaNodeType);

    if (nodeDef)
    {
        mxDataType = nodeDef->getType();
        mxNodeType = nodeDef->hasAttribute("rename") ? nodeDef->getAttribute("rename") : mayaNodeType;
        mxNodeDef  = "ND_" + mxNodeType;

        for (mx::ElementPtr child : nodeDef->getChildren())
        {
            string name = child->getName();
            string rename = child->getAttribute("rename");
            if (rename.length())
            {
                mxToMaya[rename] = name;
                name = rename;
            }

            if (child->isA<mx::Input>())
            {
                mx::InputPtr port = child->asA<mx::Input>();
                attributes.push_back(Attribute(INPUT_PORT, name, port->getType(), port->getValueString()));
            }
            else if (child->isA<mx::Parameter>())
            {
                mx::ParameterPtr port = child->asA<mx::Parameter>();
                attributes.push_back(Attribute(PARAMETER_PORT, name, port->getType(), port->getValueString()));
            }
            else if (child->isA<mx::Output>())
            {
                mx::OutputPtr port = child->asA<mx::Output>();
                attributes.push_back(Attribute(OUTPUT_PORT, name, port->getType()));
            }
        }

        if (nodeDef->hasAttribute("exportByValue"))
        {
            exportByValue = nodeDef->getAttribute("exportByValue") == "true";
        }
    }
    else
    {
        mxNodeType = mayaNodeType;
        mxNodeDef = "ND_" + mxNodeType;

        vector<Attribute> outputs;
        for (unsigned int i = 0; i < fnNode.attributeCount(); ++i)
        {
            MFnAttribute fnAttr(fnNode.attribute(i));
            const std::string attrType = SceneTranslator::getMxType(fnAttr.object());

            // Ignore unsupported types and child attributes
            if (attrType.empty() ||
                fnAttr.parent() != MObject::kNullObj)
            {
                continue;
            }

            const string name = fnAttr.name().asChar();

            if (fnAttr.isWritable())
            {
                mx::ValuePtr value = SceneTranslator::exportDefaultValue(fnAttr.object());

                if (fnAttr.isConnectable())
                {
                    attributes.push_back(Attribute(INPUT_PORT, name, attrType, value ? value->getValueString() : ""));
                }
                else
                {
                    attributes.push_back(Attribute(PARAMETER_PORT, name, attrType, value ? value->getValueString() : ""));
                }
            }
            else
            {
                outputs.push_back(Attribute(OUTPUT_PORT, name, attrType));
            }
        }

        if (outputs.size() > 1)
        {
            attributes.insert(attributes.end(), outputs.begin(), outputs.end());
            mxDataType = "multioutput";
        }
        else if (outputs.size() == 1)
        {
            mxDataType = outputs.back().type;
        }
    }
}

