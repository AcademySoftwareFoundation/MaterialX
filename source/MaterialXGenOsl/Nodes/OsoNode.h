//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_OSONODE_H
#define MATERIALX_OSONODE_H

#include <MaterialXGenOsl/Export.h>

#include <MaterialXGenShader/ShaderNodeImpl.h>

MATERIALX_NAMESPACE_BEGIN

class MX_GENOSL_API OsoNode : public ShaderNodeImpl
{
public:
    static ShaderNodeImplPtr create();

    void initialize(const InterfaceElement& element, GenContext& context) override;

    const string& getOsoName() const { return _osoName; }
    const string& getOsoPath() const { return _osoPath; }

private:
    string _osoName;
    string _osoPath;
};

MATERIALX_NAMESPACE_END

#endif
