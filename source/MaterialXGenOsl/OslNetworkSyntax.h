//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_OSLNETWORKSYNTAX_H
#define MATERIALX_OSLNETWORKSYNTAX_H

/// @file
/// OSL Network syntax class

#include <MaterialXGenOsl/Export.h>

#include <MaterialXGenShader/Syntax.h>

MATERIALX_NAMESPACE_BEGIN

/// @class OslNetworkSyntax
/// Syntax class for OSL (Open Shading Language) Network command strings
class MX_GENOSL_API OslNetworkSyntax : public Syntax
{
  public:
    OslNetworkSyntax(TypeSystemPtr typeSystem);

    static SyntaxPtr create(TypeSystemPtr typeSystem) { return std::make_shared<OslNetworkSyntax>(typeSystem); }

    const string& getOutputQualifier() const override;
    const string& getConstantQualifier() const override { return EMPTY_STRING; };
    const string& getSourceFileExtension() const override { return SOURCE_FILE_EXTENSION; };

    static const string OUTPUT_QUALIFIER;
    static const string SOURCE_FILE_EXTENSION;
    static const StringVec VECTOR_MEMBERS;
    static const StringVec VECTOR2_MEMBERS;
    static const StringVec VECTOR4_MEMBERS;
    static const StringVec COLOR4_MEMBERS;
};

MATERIALX_NAMESPACE_END

#endif
