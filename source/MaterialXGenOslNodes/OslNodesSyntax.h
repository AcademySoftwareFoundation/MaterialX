//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_OSLNODESSYNTAX_H
#define MATERIALX_OSLNODESSYNTAX_H

/// @file
/// OSL syntax class

#include <MaterialXGenOslNodes/Export.h>

#include <MaterialXGenShader/Syntax.h>

MATERIALX_NAMESPACE_BEGIN

/// @class OslSyntax
/// Syntax class for OSL (Open Shading Language)
class MX_GENOSLNODES_API OslNodesSyntax : public Syntax
{
  public:
    OslNodesSyntax(TypeSystemPtr typeSystem);

    static SyntaxPtr create(TypeSystemPtr typeSystem) { return std::make_shared<OslNodesSyntax>(typeSystem); }

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
