#ifndef MATERIALXFORMAYA_SCENETRANSLATOR_H
#define MATERIALXFORMAYA_SCENETRANSLATOR_H

#include <Types.h>

#include <maya/MStatus.h>
#include <maya/MPxFileTranslator.h>
#include <maya/MObjectHandle.h>

#include <MaterialXCore/Look.h>
#include <MaterialXCore/Document.h>

namespace mx = MaterialX;

struct TranslatorContext
{
    // The full document being exported.
    mx::DocumentPtr doc;

    // A seperate doc contaning nodedefs for non-standard 
    // node types beeing exported. Will be merged into the
    // full doc before output if requested by the user.
    mx::DocumentPtr nodeDefs;
};

class SceneTranslator
{
public:
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
    SceneTranslator(const Options& options);
    virtual ~SceneTranslator();

    mx::LookPtr exportLook(const MObjectHandleSet& mayaMaterials,
        const MObjectHandleSet& mayaLights,
        const std::string& lookName = mx::EMPTY_STRING);
    mx::MaterialPtr exportMaterial(const MObject& mayaMaterial, mx::LookPtr target);
    mx::MaterialPtr exportLight(const MObject& mayaLight, mx::LookPtr target);
    mx::ShaderRefPtr exportShader(const MObject& mayaShader, mx::MaterialPtr target);
    mx::NodeGraphPtr exportNodeGraph(const MPlug& mayaPlug, const std::string& outputType);

    static void exportValue(const MPlug& mayaPlug, mx::ValueElementPtr target);
    static mx::ValuePtr exportDefaultValue(const MObject& mayaAttr);
    static void findMaterials(bool onlySelection, MObjectHandleSet& mayaMaterials);
    static void findLights(bool onlySelection, MObjectHandleSet& mayaLights);
    static void getAssignedMaterials(const MDagPath& dagPath, MObjectHandleSet& mayaMaterials);
    static std::string getMxType(const MObject& mayaObj);

    void writeToFile(const std::string& filename);
    void writeToStream(std::ostream& stream);

private:
    void finalize();

    const Options& _options;
    TranslatorContext _context;
};

class TranslatorError : public Exception
{
public:
	TranslatorError(const string& msg) : Exception(msg) {}
};

#endif // MATERIALXFORMAYA_SCENETRANSLATOR_H
