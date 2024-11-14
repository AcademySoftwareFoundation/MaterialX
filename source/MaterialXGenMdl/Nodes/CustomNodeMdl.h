//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_CUSTOMNODEMDL_H
#define MATERIALX_CUSTOMNODEMDL_H

#include <MaterialXGenMdl/Nodes/SourceCodeNodeMdl.h>

MATERIALX_NAMESPACE_BEGIN

class MX_GENMDL_API CustomCodeNodeMdl : public SourceCodeNodeMdl
{
  public:
    static ShaderNodeImplPtr create();

    void initialize(const InterfaceElement& element, GenContext& context) override;
    const string& getQualifiedModuleName() const;

  protected:
    string _qualifiedModuleName;
};

MATERIALX_NAMESPACE_END

#endif
