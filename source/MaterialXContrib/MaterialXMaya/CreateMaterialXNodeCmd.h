#ifndef CREATE_MATERIALX_NODE_CMD_H
#define CREATE_MATERIALX_NODE_CMD_H

#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>

#include <MaterialXCore/Document.h>
#include <MaterialXRender/Util.h>

#include <vector>

namespace mx = MaterialX;

namespace MaterialXMaya
{

///
///
///
class CreateMaterialXNodeCmd : MPxCommand
{
  public:
    CreateMaterialXNodeCmd();
    ~CreateMaterialXNodeCmd() override;

    MStatus doIt(const MArgList&) override;
    bool isUndoable() { return false; }

    static MSyntax newSyntax();
    static void* creator();

    static MString NAME;

  private:
    /// Create a new Maya node for a given renderable element
    /// @param document Document containing the element
    /// @param renderableElement Element to use
    /// @param createAsTexture Create texture node. If set to false will type found based on associated shader code.
    /// @param documentFilePath Path to document
    /// @param searchPath Shader generation source paths
    /// @return Name of Maya node created
    std::string createNode(
        mx::DocumentPtr document,
        mx::TypedElementPtr renderableElement,
        bool createAsTexture,
        const MString& documentFilePath,
        const mx::FileSearchPath& searchPath,
        const MString& envRadianceFileName,
        const MString& envIrradianceFileName );

    MDGModifier _dgModifier;
};

} // namespace MaterialXMaya

#endif /* CREATE_MATERIALX_NODE_CMD_H */
