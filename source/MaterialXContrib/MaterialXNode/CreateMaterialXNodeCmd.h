#ifndef CREATE_MATERIALX_NODE_CMD_H
#define CREATE_MATERIALX_NODE_CMD_H

#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>

#include <MaterialXCore/Document.h>
#include <MaterialXRender/Util.h>

#include <vector>

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
    MDGModifier _dgModifier;
};

#endif /* CREATE_MATERIALX_NODE_CMD_H */
