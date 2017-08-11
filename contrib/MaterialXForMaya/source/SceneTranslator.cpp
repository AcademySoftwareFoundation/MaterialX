#include <SceneTranslator.h>
#include <NodeTranslator.h>
#include <Plugin.h>

#include <MaterialXCore/Value.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/File.h>

#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MDagPath.h>
#include <maya/MItDag.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnSet.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MObject.h>
#include <maya/MObjectHandle.h>
#include <maya/MFnLight.h>

namespace
{
    static const std::set<std::string> kMayaFilenamePlugs =
    {
        "fileTextureName"
    };

    static const std::vector<MString> kMayaOutputPlugs =
    {
        "outColor",
        "outAlpha",
        "outNormal",
        "outUV",
        "output",
        "out"
    };

    static const std::string kDefaultGraphOutputName = "out";
}

SceneTranslator::SceneTranslator(const Options& options)
    : _options(options)
{
    _context.doc = mx::createDocument();
    _context.nodeDefs = mx::createDocument();

    // Load stdlib if requested
    if (options.includeStdLib)
    {
        mx::DocumentPtr stdlib = mx::createDocument();
        mx::readFromXmlFile(stdlib, "mx_stdlib.mtlx", Plugin::instance().getLibrarySearchPath());
        _context.doc->importLibrary(stdlib);
    }
}

SceneTranslator::~SceneTranslator()
{
}

mx::LookPtr SceneTranslator::exportLook(const MObjectHandleSet& mayaMaterials,
                                const MObjectHandleSet& mayaLights,
                                const std::string& lookName)
{
    mx::LookPtr look = _context.doc->getLook(lookName);
    if (!look)
    {
        look = _context.doc->addLook(lookName);
    }

    for (MObjectHandle mat : mayaMaterials)
    {
        exportMaterial(mat.object(), look);
    }
    for (MObjectHandle light : mayaLights)
    {
        exportLight(light.object(), look);
    }

    return look;
}

mx::MaterialPtr SceneTranslator::exportLight(const MObject& mayaLight, mx::LookPtr target)
{
    // - Adds a material
    // - Assign the material to the light DAG object name
    // - Adds visibility assignment to geometry (default all geometry) if option enabled
    // - Adds shader if option enabled
    // - 
    MFnDagNode fnLight(mayaLight);
    const std::string lightName = fnLight.name().asChar();
    const std::string lightPathName = fnLight.fullPathName().asChar();

    // Light is a material
    mx::MaterialPtr material = _context.doc->getMaterial(lightName);
    if (material)
    {
        // Already exported.
        return material;
    }

    material = _context.doc->addMaterial(lightName);

    // Assign material to light object
    mx::MaterialAssignPtr materialAssign = target->addMaterialAssign();
    materialAssign->setMaterial(lightName);
    materialAssign->setGeom(lightPathName);

    // Light association is a visibility. No light linking
    if (_options.lightAssignments)
    {
        MaterialX::VisibilityPtr visibility = target->addVisibility(lightName);
        visibility->setGeom(lightPathName);
        visibility->setViewerGeom("*");
        visibility->setVisible(true);
    }

    if (_options.lightShaders)
    {
        exportShader(mayaLight, material);
    }

    return nullptr;
}

mx::MaterialPtr SceneTranslator::exportMaterial(const MObject& mayaMaterial, mx::LookPtr target)
{
    MFnSet fnMaterial(mayaMaterial);

    mx::MaterialPtr material = _context.doc->getMaterial(fnMaterial.name().asChar());
    if (material)
    {
        // Already exported.
        return material;
    }

    material = _context.doc->addMaterial(fnMaterial.name().asChar());

    if (_options.materialAssignments)
    {
        // Create all material assignments.
        MSelectionList members;
        fnMaterial.getMembers(members, false);

        mx::CollectionPtr collection = _context.doc->addCollection();
        mx::MaterialAssignPtr materialAssign = target->addMaterialAssign();
        materialAssign->setMaterial(material->getName());
        materialAssign->setCollection(collection->getName());
        for (unsigned int j = 0; j < members.length(); ++j)
        {
            MDagPath dagPath;
            members.getDagPath(j, dagPath);
            mx::CollectionAddPtr collAdd = collection->addCollectionAdd();
            collAdd->setGeom(dagPath.fullPathName().asChar());
        }
    }

    if (_options.surfaceShaders)
    {
        MPlug surfaceShaderSrc = fnMaterial.findPlug("surfaceShader", false).source();
        if (!surfaceShaderSrc.isNull())
        {
            exportShader(surfaceShaderSrc.node(), material);
        }
    }

    if (_options.displacementShaders)
    {
        MPlug displacementShaderSrc = fnMaterial.findPlug("displacementShader", false).source();
        if (!displacementShaderSrc.isNull())
        {
            exportShader(displacementShaderSrc.node(), material);
        }
    }

    return material;
}

mx::ShaderRefPtr SceneTranslator::exportShader(const MObject& mayaShader, mx::MaterialPtr target)
{
    MFnDependencyNode fnShader(mayaShader);

    const std::string shaderName = fnShader.name().asChar();
    mx::ShaderRefPtr shaderRef = target->getShaderRef(shaderName);
    if (shaderRef)
    {
        // Already exported.
        return shaderRef;
    }

    NodeTranslatorPtr translator = Plugin::instance().getTranslator(mayaShader);

    const string& shaderType = getMxType(mayaShader);
    mx::NodeDefPtr nodeDef = translator->exportNodeDef(mayaShader, shaderType, _context);
    if (!nodeDef)
    {
        MGlobal::displayWarning(("Can't find a nodedef for shader " + shaderName + ".").c_str());
        return nullptr;
    }

    shaderRef = target->addShaderRef(shaderName);
    shaderRef->setAttribute("node", nodeDef->getNode());

    // Export inputs
    for (mx::InputPtr input : nodeDef->getInputs())
    {
        const string mayaName = translator->getMayaName(input->getName());
        MPlug plug = fnShader.findPlug(mayaName.c_str(), false);
        if (translator->shouldExport(plug, input->getValue()))
        {
            mx::BindInputPtr bindInput = shaderRef->addBindInput(input->getName(), input->getType());

            bool exportByValue = true;
            if (plug.isDestination())
            {
                NodeTranslatorPtr srcNodeTranslator = Plugin::instance().getTranslator(plug.source().node());
                if (!srcNodeTranslator->exportByValue())
                {
                    std::string outputName;
                    mx::NodeGraphPtr graph = exportNodeGraph(plug.source(), input->getType());
                    mx::OutputPtr output = graph ? graph->getOutput(kDefaultGraphOutputName) : nullptr;
                    if (output)
                    {
                        bindInput->setConnectedOutput(output);
                        exportByValue = false;
                    }
                }
            }
            if (exportByValue)
            {
                exportValue(plug, bindInput);
            }
        }
    }

    // Export parameters
    for (mx::ParameterPtr param : nodeDef->getParameters())
    {
        const string mayaName = translator->getMayaName(param->getName());
        MPlug plug = fnShader.findPlug(mayaName.c_str(), false);
        if (translator->shouldExport(plug, param->getValue()))
        {
            mx::BindParamPtr bindParam = shaderRef->addBindParam(param->getName(), param->getType());
            exportValue(plug, bindParam);
        }
    }

    return shaderRef;
}

mx::NodeGraphPtr SceneTranslator::exportNodeGraph(const MPlug& mayaPlug, const std::string& outputType)
{
    std::string graphName = mayaPlug.name().asChar();
    std::replace(graphName.begin(), graphName.end(), '.', '_');

    mx::NodeGraphPtr graph = _context.doc->getNodeGraph(graphName);
    if (graph)
    {
        return graph;
    }

    graph = _context.doc->addNodeGraph(graphName);
    mx::OutputPtr output = graph->addOutput(kDefaultGraphOutputName, outputType);

    NodeTranslatorPtr translator = Plugin::instance().getTranslator(mayaPlug.node());

    mx::NodePtr node = translator->exportNode(mayaPlug.node(), outputType, graph, _context);
    if (node)
    {
        output->setConnectedNode(node);
        if (node->getType() == "multioutput")
        {
            const std::string srcOutput = mayaPlug.partialName(false, false, false, false, false, true).asChar();
            output->setAttribute("output", srcOutput);
        }
    }

    return graph;
}

void SceneTranslator::exportValue(const MPlug& mayaPlug, mx::ValueElementPtr target)
{
    const std::string mxType = getMxType(mayaPlug.attribute());
    if (mxType == "boolean")
    {
        target->setValue(mayaPlug.asBool());
    }
    else if (mxType == "integer")
    {
        target->setValue(mayaPlug.asInt());
    }
    else if (mxType == "float")
    {
        target->setValue(mayaPlug.asFloat());
    }
    else if (mxType == "vector2")
    {
        target->setValue(mx::Vector2(mayaPlug.child(0).asFloat(), mayaPlug.child(1).asFloat()));
    }
    else if (mxType == "color3")
    {
        target->setValue(mx::Color3(mayaPlug.child(0).asFloat(), mayaPlug.child(1).asFloat(), mayaPlug.child(2).asFloat()));
    }
    else if (mxType == "vector3")
    {
        target->setValue(mx::Vector3(mayaPlug.child(0).asFloat(), mayaPlug.child(1).asFloat(), mayaPlug.child(2).asFloat()));
    }
    else if (mxType == "color4")
    {
        target->setValue(mx::Color4(mayaPlug.child(0).asFloat(), mayaPlug.child(1).asFloat(), mayaPlug.child(2).asFloat(), mayaPlug.child(3).asFloat()));
    }
    else if (mxType == "vector4")
    {
        target->setValue(mx::Vector4(mayaPlug.child(0).asFloat(), mayaPlug.child(1).asFloat(), mayaPlug.child(2).asFloat(), mayaPlug.child(3).asFloat()));
    }
    else if (mxType == "string" || mxType == "filename")
    {
        target->setValueString(mayaPlug.asString().asChar());
    }
}

mx::ValuePtr SceneTranslator::exportDefaultValue(const MObject& mayaAttr)
{
    const std::string mxType = getMxType(mayaAttr);
    if (mxType == "boolean")
    {
        MFnNumericAttribute fnAttr(mayaAttr);
        bool value;
        fnAttr.getDefault(value);
        return mx::Value::createValue(value);
    }
    else if (mxType == "integer")
    {
        MFnNumericAttribute fnAttr(mayaAttr);
        int value;
        fnAttr.getDefault(value);
        return mx::Value::createValue(value);
    }
    else if (mxType == "float")
    {
        MFnNumericAttribute fnAttr(mayaAttr);
        float value;
        fnAttr.getDefault(value);
        return mx::Value::createValue(value);
    }
    else if (mxType == "vector2")
    {
        MFnNumericAttribute fnAttr(mayaAttr);
        float x, y;
        fnAttr.getDefault(x, y);
        return mx::Value::createValue(mx::Vector2(x, y));
    }
    else if (mxType == "color3")
    {
        MFnNumericAttribute fnAttr(mayaAttr);
        float x, y, z;
        fnAttr.getDefault(x, y, z);
        return mx::Value::createValue(mx::Color3(x, y, z));
    }
    else if (mxType == "vector3")
    {
        MFnNumericAttribute fnAttr(mayaAttr);
        float x, y, z;
        fnAttr.getDefault(x, y, z);
        return mx::Value::createValue(mx::Vector3(x, y, z));
    }
    else if (mxType == "color4")
    {
        MFnNumericAttribute fnAttr(mayaAttr);
        double x, y, z, w;
        fnAttr.getDefault(x, y, z, w);
        return mx::Value::createValue(mx::Color4(float(x), float(y), float(z), float(w)));
    }
    else if (mxType == "vector4")
    {
        MFnNumericAttribute fnAttr(mayaAttr);
        double x, y, z, w;
        fnAttr.getDefault(x, y, z, w);
        return mx::Value::createValue(mx::Vector4(float(x), float(y), float(z), float(w)));
    }
    else if (mxType == "string" || mxType == "filename")
    {
        MFnTypedAttribute fnAttr(mayaAttr);
        MObject strData;
        fnAttr.getDefault(strData);
        MFnStringData fnStrData(strData);
        return mx::Value::createValue<string>(fnStrData.string().asChar());
    }

    return nullptr;
}

void SceneTranslator::findMaterials(bool onlySelection, MObjectHandleSet& mayaMaterials)
{
    if (onlySelection)
    {
        // Create an iterator for the active selection list.
        MSelectionList slist;
        MGlobal::getActiveSelectionList(slist);
        MItSelectionList iter(slist);

        if (iter.isDone())
        {
            throw TranslatorError("Nothing is selected.");
        }

        // Selection list loop.
        for (; !iter.isDone(); iter.next())
        {
            if (iter.itemType() == MItSelectionList::kDagSelectionItem)
            {
                MObject node;
                MStatus status = iter.getDependNode(node);
                if (!status)
                {
                    throw TranslatorError("Error getting DAG path from selected object");
                }

                // We will need to interate over the dag heirarchy to find all material assignments
                MItDag dagIterator(MItDag::kDepthFirst, MFn::kInvalid);
                dagIterator.reset(node, MItDag::kDepthFirst, MFn::kInvalid);
                for (; !dagIterator.isDone(); dagIterator.next())
                {
                    MDagPath dagPath;
                    status = dagIterator.getPath(dagPath);
                    if (!status)
                    {
                        throw TranslatorError("Error getting DAG path from selected object");
                    }

                    getAssignedMaterials(dagPath, mayaMaterials);
                }
            }
            else if (iter.itemType() == MItSelectionList::kDNselectionItem)
            {
                MObject node;
                MStatus status = iter.getDependNode(node);
                if (!status)
                {
                    throw TranslatorError("Error getting dependency node from selected object");
                }

                // Check if the node is a Maya material.
                if (node.hasFn(MFn::kShadingEngine))
                {
                    mayaMaterials.insert(node);
                }
            }
        }
    }
    else
    {
        MStringArray sets;
        MGlobal::executeCommand("ls -type shadingEngine", sets);

        for (unsigned int i = 0; i < sets.length(); ++i)
        {
            MSelectionList list;
            list.add(sets[i]);

            MObject obj;
            list.getDependNode(0, obj);
            MFnSet fnSet(obj);

            MSelectionList members;
            fnSet.getMembers(members, false);
            if (members.length() > 0)
            {
                mayaMaterials.insert(obj);
            }
        }
    }
}

void SceneTranslator::getAssignedMaterials(const MDagPath& dagPath, MObjectHandleSet& mayaMaterials)
{
    unsigned int instanceNr = dagPath.instanceNumber();
    MFnDependencyNode fnNode(dagPath.node());

    MPlug instObjGroups = fnNode.findPlug("instObjGroups", false);
    if (instanceNr >= instObjGroups.evaluateNumElements())
    {
        return;
    }

    MPlugArray plugs;

    // Get the whole instance
    MPlug instObjGroup = instObjGroups.elementByLogicalIndex(instanceNr);
    plugs.append(instObjGroup);

    // Get per face plugs
    MPlug objGroups = instObjGroup.child(0);
    for (unsigned int i = 0; i < objGroups.evaluateNumElements(); ++i)
    {
        plugs.append(objGroups.elementByLogicalIndex(i));
    }

    // Find connected shading engines
    for (unsigned int i = 0; i < plugs.length(); ++i)
    {
        const MPlug& src = plugs[i];
        MPlugArray destinations;
        src.destinations(destinations);
        for (unsigned int j = 0; j < destinations.length(); ++j)
        {
            const MPlug& dest = destinations[j];
            if (dest.node().hasFn(MFn::kShadingEngine))
            {
                mayaMaterials.insert(dest.node());
            }
        }
    }
}

void SceneTranslator::findLights(bool onlySelection, MObjectHandleSet& mayaLights)
{
    MStringArray lights;
    if (onlySelection)
    {
        MGlobal::executeCommand("ls -sl -type `listNodeTypes(\"light\")`", lights);
    }
    else
    {
        MGlobal::executeCommand("ls -type `listNodeTypes(\"light\")`", lights);
    }
    for (unsigned int i = 0; i < lights.length(); ++i)
    {
        MSelectionList list;
        list.add(lights[i]);

        MObject obj;
        list.getDependNode(0, obj);

        mayaLights.insert(obj);
    }
}

std::string SceneTranslator::getMxType(const MObject& mayaObj)
{
    if (mayaObj.hasFn(MFn::kDependencyNode))
    {
        MFnDependencyNode fnNode(mayaObj);
        int result;
        MGlobal::executeCommand("getClassification -satisfies \"shader/surface\" " + fnNode.typeName(), result);
        if (result != 0)
        {
            return "surfaceshader";
        }
        MGlobal::executeCommand("getClassification -satisfies \"shader/displacement\" " + fnNode.typeName(), result);
        if (result != 0)
        {
            return "displacementshader";
        }
        MGlobal::executeCommand("getClassification -satisfies \"light\" " + fnNode.typeName(), result);
        if (result != 0)
        {
            return "lightshader";
        }

        for (const MString& plugName : kMayaOutputPlugs)
        {
            MPlug outPlug = fnNode.findPlug(plugName, false);
            if (!outPlug.isNull())
            {
                return getMxType(outPlug.attribute());
            }
        }
    }
    else if (mayaObj.hasFn(MFn::kNumericAttribute))
    {
        MFnNumericAttribute fnAttr(mayaObj);
        switch (fnAttr.unitType())
        {
            case MFnNumericData::kBoolean:
                return "boolean";
            case MFnNumericData::kInt:
            case MFnNumericData::kShort:
            case MFnNumericData::kChar:
            case MFnNumericData::kByte:
                return "integer";
            case MFnNumericData::kFloat:
            case MFnNumericData::kDouble:
                return "float";
            case MFnNumericData::k2Float:
            case MFnNumericData::k2Double:
            case MFnNumericData::k2Int:
            case MFnNumericData::k2Short:
                return "vector2";
            case MFnNumericData::k3Float:
            case MFnNumericData::k3Double:
            case MFnNumericData::k3Int:
            case MFnNumericData::k3Short:
                return fnAttr.isUsedAsColor() ? "color3" : "vector3";
            case MFnNumericData::k4Double:
                return fnAttr.isUsedAsColor() ? "color4" : "vector4";
            default:
                break;
        }
    }
    else if (mayaObj.hasFn(MFn::kEnumAttribute))
    {
        return "integer";
    }  
    else if (mayaObj.hasFn(MFn::kTypedAttribute))
    {
        MFnTypedAttribute fnAttr(mayaObj);
        switch (fnAttr.attrType())
        {
            case MFnData::kString:
                return (kMayaFilenamePlugs.find(fnAttr.name().asChar()) != kMayaFilenamePlugs.end() ? "filename" : "string");
            case MFnData::kMatrix:
                return "matrix44";
            default:
                break;
        }
    }

    return "";
}

void SceneTranslator::finalize()
{
    if (_options.nodeDefinitions)
    {
        _context.doc->importLibrary(_context.nodeDefs);
        _context.nodeDefs->clearContent();
    }
    _context.doc->generateRequireString();
}

void SceneTranslator::writeToFile(const std::string& filename)
{
    finalize();
    mx::writeToXmlFile(_context.doc, filename, true);
}

void SceneTranslator::writeToStream(std::ostream& stream)
{
    finalize();
    mx::writeToXmlStream(_context.doc, stream, true);
}
