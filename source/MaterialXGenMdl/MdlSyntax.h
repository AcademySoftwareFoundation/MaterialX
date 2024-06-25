//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_MDLSYNTAX_H
#define MATERIALX_MDLSYNTAX_H

/// @file
/// MDL syntax class

#include <MaterialXGenMdl/Export.h>

#include <MaterialXGenShader/Syntax.h>

MATERIALX_NAMESPACE_BEGIN

class MdlSyntax;

/// Shared pointer to an MdlSyntax
using MdlSyntaxPtr = shared_ptr<MdlSyntax>;

/// @class MdlSyntax
/// Syntax class for MDL (Material Definition Language)
class MX_GENMDL_API MdlSyntax : public Syntax
{
  public:
    MdlSyntax();

    static SyntaxPtr create() { return std::make_shared<MdlSyntax>(); }

    const string& getConstantQualifier() const override { return CONST_QUALIFIER; };
    const string& getUniformQualifier() const override { return UNIFORM_QUALIFIER; };
    const string& getSourceFileExtension() const override { return SOURCE_FILE_EXTENSION; };

    /// Override to return array type suffix.
    string getArrayTypeSuffix(TypeDesc type, const Value& value) const override;

    /// Override to indicate array variables have no array suffix.
    string getArrayVariableSuffix(TypeDesc, const Value&) const override { return EMPTY_STRING; };

    static const string CONST_QUALIFIER;
    static const string UNIFORM_QUALIFIER;
    static const string SOURCE_FILE_EXTENSION;
    static const StringVec VECTOR2_MEMBERS;
    static const StringVec VECTOR3_MEMBERS;
    static const StringVec VECTOR4_MEMBERS;
    static const StringVec COLOR3_MEMBERS;
    static const StringVec COLOR4_MEMBERS;
    static const StringVec ADDRESSMODE_MEMBERS;
    static const StringVec COORDINATESPACE_MEMBERS;
    static const StringVec FILTERLOOKUPMODE_MEMBERS;
    static const StringVec FILTERTYPE_MEMBERS;
    static const StringVec DISTRIBUTIONTYPE_MEMBERS;
    static const StringVec SCATTER_MODE_MEMBERS;

    /// Get an type description for an enumeration based on member value
    TypeDesc getEnumeratedType(const string& value) const;

    /// Given an input specification attempt to remap this to an enumeration which is accepted by
    /// the shader generator. The enumeration may be converted to a different type than the input.
    bool remapEnumeration(const string& value, TypeDesc type, const string& enumNames, std::pair<TypeDesc, ValuePtr>& result) const override;

    /// Modify the given name string to remove any invalid characters or tokens.
    void makeValidName(string& name) const override;
};

namespace Type
{

TYPEDESC_DEFINE_TYPE(MDL_COORDINATESPACE, "coordinatespace", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_ENUM, 0)
TYPEDESC_DEFINE_TYPE(MDL_ADDRESSMODE, "addressmode", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_ENUM, 0)
TYPEDESC_DEFINE_TYPE(MDL_FILTERLOOKUPMODE, "filterlookup", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_ENUM, 0)
TYPEDESC_DEFINE_TYPE(MDL_FILTERTYPE, "filtertype", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_ENUM, 0)
TYPEDESC_DEFINE_TYPE(MDL_DISTRIBUTIONTYPE, "distributiontype", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_ENUM, 0)
TYPEDESC_DEFINE_TYPE(MDL_SCATTER_MODE, "scatter_mode", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_ENUM, 0)

} // namespace Type

MATERIALX_NAMESPACE_END

#endif
