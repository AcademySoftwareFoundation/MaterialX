//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SLANGTYPEUTILS_H
#define MATERIALX_SLANGTYPEUTILS_H

#include <MaterialXRenderSlang/Export.h>
#include <MaterialXRenderSlang/SlangRhi.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXRender/Image.h>

MATERIALX_NAMESPACE_BEGIN

/// Returns the number of components and base type when the RHI Format can be mapped to Image.
/// When it cannot be mapped, returns 0 components.
std::pair<unsigned int, Image::BaseType> getImageConfig(rhi::Format format);

rhi::Format getRHIFormat(Image::BaseType baseType, unsigned int channelCount, bool useSrgb = false);

/// Convert a slang type to the corresponding buffer format, used for vertex input layout
rhi::Format getRHIFormat(slang::TypeReflection* slangType);

/// True when the slangType is `SamplerTexture2D`, the Texture2D + SamplerState
bool isSamplerTexture2D(slang::TypeReflection* type);
inline bool isSamplerTexture2D(slang::TypeLayoutReflection* typeLayout)
{
    return isSamplerTexture2D(typeLayout->getType());
}

/// Returns true if the slangType is a string (found in User defined attributes for DefaultValue)
bool isSlangString(slang::TypeReflection* slangType);

/// Returns true for types that either are not structs, or are scalar-like structs (e.g. SamplerTexture2D)
bool isScalarLikeType(slang::TypeReflection* type);
inline bool isScalarLikeType(slang::TypeLayoutReflection* typeLaytout)
{
    return isScalarLikeType(typeLaytout->getType());
}

/// Returns true if the type in Slang and in the MxType matches.
bool isEqualType(slang::TypeReflection* slangType, TypeDesc mxType);
inline bool isEqualType(slang::TypeLayoutReflection* slangTypeLayout, TypeDesc mxType)
{
    return isEqualType(slangTypeLayout->getType(), std::move(mxType));
}

// Returns full name of the slang type, e.g., `vector<4, float>`
string getFullName(slang::TypeReflection* type);
inline string getFullName(slang::TypeLayoutReflection* typeLayout)
{
    return getFullName(typeLayout->getType());
}

// Returns byte size of the slang type on the GPU, used for vertex inputs buffer layout
size_t getByteSize(slang::TypeLayoutReflection* typeLayout);

/// Set Slang variable to the value.
void setSlangValue(const rhi::ShaderCursor& cursor, const ConstValuePtr& value, size_t slangByteSize);
inline void setSlangValue(const rhi::ShaderCursor& cursor, const ConstValuePtr& value)
{
    setSlangValue(cursor, value, getByteSize(cursor.getTypeLayout()));
}

/// Clear Slang variable with zeros.
void clearSlangValue(const rhi::ShaderCursor& cursor, size_t slangByteSize);
inline void clearSlangValue(const rhi::ShaderCursor& cursor)
{
    clearSlangValue(cursor, getByteSize(cursor.getTypeLayout()));
}

MATERIALX_NAMESPACE_END

#endif
