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
    MdlSyntax(TypeSystemPtr typeSystem);

    static SyntaxPtr create(TypeSystemPtr typeSystem) { return std::make_shared<MdlSyntax>(typeSystem); }

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
    static const StringVec SHEEN_MODE_MEMBERS;
    static const string PORT_NAME_PREFIX; // Applied to input and output names to avoid collisions with reserved words in MDL

    /// Get an type description for an enumeration based on member value
    TypeDesc getEnumeratedType(const string& value) const;

    /// Given an input specification attempt to remap this to an enumeration which is accepted by
    /// the shader generator. The enumeration may be converted to a different type than the input.
    bool remapEnumeration(const string& value, TypeDesc type, const string& enumNames, std::pair<TypeDesc, ValuePtr>& result) const override;

    /// Modify the given name string to remove any invalid characters or tokens.
    void makeValidName(string& name) const override;

    /// To avoid collisions with reserved names in MDL, input and output names are prefixed.
    string modifyPortName(const string& word) const;

    /// Replaces all markers in a source code string indicated by {{...}}.
    /// The replacement is defined by a callback function.
    string replaceSourceCodeMarkers(const string& nodeName, const string& soureCode, std::function<string(const string&)> lambda) const;

    /// Get the MDL language versing marker: {{MDL_VERSION_SUFFIX}}.
    const string& getMdlVersionSuffixMarker() const;
};

namespace Type
{

TYPEDESC_DEFINE_TYPE(MDL_COORDINATESPACE, "coordinatespace", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_ENUM, 0)
TYPEDESC_DEFINE_TYPE(MDL_ADDRESSMODE, "addressmode", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_ENUM, 0)
TYPEDESC_DEFINE_TYPE(MDL_FILTERLOOKUPMODE, "filterlookup", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_ENUM, 0)
TYPEDESC_DEFINE_TYPE(MDL_FILTERTYPE, "filtertype", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_ENUM, 0)
TYPEDESC_DEFINE_TYPE(MDL_DISTRIBUTIONTYPE, "distributiontype", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_ENUM, 0)
TYPEDESC_DEFINE_TYPE(MDL_SCATTER_MODE, "scatter_mode", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_ENUM, 0)
TYPEDESC_DEFINE_TYPE(MDL_SHEEN_MODE, "mode", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_ENUM, 0)

} // namespace Type

MATERIALX_NAMESPACE_END

#endif
