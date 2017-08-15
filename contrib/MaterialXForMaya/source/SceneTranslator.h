// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#ifndef MATERIALXFORMAYA_SCENETRANSLATOR_H
#define MATERIALXFORMAYA_SCENETRANSLATOR_H

#include <ExporterTypes.h>

#include <maya/MStatus.h>
#include <maya/MPxFileTranslator.h>
#include <maya/MObjectHandle.h>

#include <MaterialXCore/Look.h>
#include <MaterialXCore/Document.h>

namespace mx = MaterialX;

namespace MaterialXForMaya
{

/// @struct TranslatorContext
/// Context for translation
struct TranslatorContext
{
    /// The full document being exported.
    mx::DocumentPtr doc;

    /// A seperate doc contaning nodedefs for non-standard 
    /// node types beeing exported. Will be merged into the
    /// full doc before output if requested by the user.
    mx::DocumentPtr nodeDefs;
};

/// @class SceneTranslator
/// Maya scene translator
class SceneTranslator
{
public:
    /// @struct SceneTranslator::Options
    /// Options for scene export
    struct Options
    {
        bool includeStdLib;
        bool materialAssignments;
        bool surfaceShaders;
        bool displacementShaders;
        bool nodeDefinitions;
        bool lights;
        bool lightAssignments;
        bool lightShaders;
    };

    struct MObjectHandleLess {
        bool operator()(const MObjectHandle &lhs, const MObjectHandle &rhs) {
            return lhs.hashCode() < rhs.hashCode();
        }
    };
    using MObjectHandleSet = std::set<MObjectHandle, MObjectHandleLess>;

public:
    /// Scene translator constructor
    SceneTranslator(const Options& options);
    /// Scene translator destructor
    virtual ~SceneTranslator();

    /// Look export
    mx::LookPtr exportLook(const MObjectHandleSet& mayaMaterials,
        const MObjectHandleSet& mayaLights,
        const std::string& lookName = mx::EMPTY_STRING);
    /// Material export
    mx::MaterialPtr exportMaterial(const MObject& mayaMaterial, mx::LookPtr target);
    /// Light export
    mx::MaterialPtr exportLight(const MObject& mayaLight, mx::LookPtr target);
    /// Shader export
    mx::ShaderRefPtr exportShader(const MObject& mayaShader, mx::MaterialPtr target);
    /// Node graph export
    mx::NodeGraphPtr exportNodeGraph(const MPlug& mayaPlug, const std::string& outputType);

    /// Export a value on a node attribute
    static void exportValue(const MPlug& mayaPlug, mx::ValueElementPtr target);
    /// Export a default value of an attribute
    static mx::ValuePtr exportDefaultValue(const MObject& mayaAttr);
    
    /// Scan for selected or all materials
    static void findMaterials(bool onlySelection, MObjectHandleSet& mayaMaterials);
    /// Scan for selected or all lights
    static void findLights(bool onlySelection, MObjectHandleSet& mayaLights);
    /// Find materials for an objects
    static void getAssignedMaterials(const MDagPath& dagPath, MObjectHandleSet& mayaMaterials);
    /// Determine the MaterialX type for an object
    static std::string getMaterialXType(const MObject& mayaObj);

    /// Write to a file
    void writeToFile(const std::string& filename);
    /// Write to a stream
    void writeToStream(std::ostream& stream);

private:
    void finalize();

    const Options& _options;
    TranslatorContext _context;
};

/// @class TranslatorError
/// Handler for translation exceptions
class TranslatorError : public Exception
{
public:
	TranslatorError(const string& msg) : Exception(msg) {}
};

} //namespace MaterialXForMaya

#endif // MATERIALXFORMAYA_SCENETRANSLATOR_H
