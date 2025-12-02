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

    StructTypeSyntaxPtr createStructSyntax(const string& structTypeName, const string& defaultValue,
                                           const string& uniformDefaultValue, const string& typeAlias,
                                           const string& typeDefinition) const override;

    const string& getOutputQualifier() const override { return EMPTY_STRING; };
    const string& getConstantQualifier() const override { return EMPTY_STRING; };
    const string& getSourceFileExtension() const override { return EMPTY_STRING; };

    static const StringVec VECTOR_MEMBERS;
    static const StringVec VECTOR2_MEMBERS;
    static const StringVec VECTOR4_MEMBERS;
    static const StringVec COLOR4_MEMBERS;
};

/// Interface class for exporting OSL commands for parameters
class MX_GENOSL_API OslNetworkSyntaxEmit
{
public:
    struct EmitParamPart
    {
        EmitParamPart(const string& type, const string& name, const string& value) :
            typeName(type), paramName(name), paramValue(value) { }
        string typeName;
        string paramName;
        string paramValue;
    };
    using EmitParamPartVec = vector<EmitParamPart>;

    virtual EmitParamPartVec getEmitParamParts(const string& name, TypeDesc typeDesc, const Value& value) const = 0;
};


/// Specialization of TypeSyntax for struct types.
class MX_GENOSL_API OslNetworkStructTypeSyntax : public StructTypeSyntax, public OslNetworkSyntaxEmit
{
public:
    OslNetworkStructTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                     const string& typeAlias = EMPTY_STRING, const string& typeDefinition = EMPTY_STRING,
                     const StringVec& members = EMPTY_MEMBERS);
    virtual ~OslNetworkStructTypeSyntax();

    EmitParamPartVec getEmitParamParts(const string& name, TypeDesc typeDesc, const Value& value) const override;
};

MATERIALX_NAMESPACE_END

#endif
