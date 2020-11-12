#ifndef MATERIALX_MAYA_CREATENODECMD_H
#define MATERIALX_MAYA_CREATENODECMD_H

/// @file
/// Maya command for creating MaterialX shading nodes.

#include <MaterialXCore/Document.h>
#include <MaterialXRender/Util.h>
#include <maya/MDGModifier.h>
#include <maya/MPxCommand.h>

#include <vector>

namespace mx = MaterialX;

namespace MaterialXMaya
{

/// @class CreateMaterialXNodeCmd
/// Creates one or more MaterialX nodes from the specified MaterialX document.
///
class CreateMaterialXNodeCmd : MPxCommand
{
  public:
    CreateMaterialXNodeCmd();
    ~CreateMaterialXNodeCmd() override;

    /// @name Maya API methods
    /// @{
    MStatus doIt(const MArgList&) override;
    bool isUndoable() const override
    {
        return false;
    }

    static MSyntax newSyntax();
    static void* creator();
    /// @}

    /// The name of the command in MEL
    static MString NAME;

  private:
    /// Specifies the type of shading node to create
    enum class NodeTypeToCreate
    {
        AUTO,    ///< Determined by the type of the MaterialX element
        SURFACE, ///< A surface shading node
        TEXTURE  ///< A texture shading node
    };

    /// Create a new Maya node for a given renderable element.
    /// @param renderableElement The element to use.
    /// @param nodeTypeToCreate The type of shading node to create.
    /// @param documentFilePath Path to the document.
    /// @param searchPath Shader generation source paths.
    /// @param envRadianceFileName The file name of the environment map to use for specular shading.
    /// @param envIrradianceFileName The file name of the environment map to use for diffuse shading.
    /// @return Name of Maya node created.
    ///
    std::string createNode(mx::TypedElementPtr renderableElement,
                           NodeTypeToCreate nodeTypeToCreate,
                           const MString& documentFilePath,
                           const mx::FileSearchPath& searchPath,
                           const MString& envRadianceFileName,
                           const MString& envIrradianceFileName);

    /// Used to make the necessary changes to Maya's dependency graph.
    MDGModifier _dgModifier;
};

} // namespace MaterialXMaya

#endif /* CREATE_MATERIALX_NODE_CMD_H */
