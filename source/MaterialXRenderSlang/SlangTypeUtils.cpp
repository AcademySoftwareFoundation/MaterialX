//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderSlang/SlangTypeUtils.h>
#include <MaterialXRender/ShaderRenderer.h>

MATERIALX_NAMESPACE_BEGIN

std::pair<unsigned int, Image::BaseType> getImageConfig(rhi::Format format)
{
    switch (format)
    {
        case rhi::Format::Undefined:
        case rhi::Format::R8Uint:
        case rhi::Format::R8Unorm:
            return { 1, Image::BaseType::UINT8 };
        case rhi::Format::R8Sint:
        case rhi::Format::R8Snorm:
            return { 1, Image::BaseType::INT8 };

        case rhi::Format::RG8Uint:
        case rhi::Format::RG8Unorm:
            return { 2, Image::BaseType::UINT8 };
        case rhi::Format::RG8Sint:
        case rhi::Format::RG8Snorm:
            return { 2, Image::BaseType::INT8 };

        case rhi::Format::RGBA8Uint:
        case rhi::Format::RGBA8Unorm:
        case rhi::Format::RGBA8UnormSrgb:
            return { 4, Image::BaseType::UINT8 };
        case rhi::Format::RGBA8Sint:
        case rhi::Format::RGBA8Snorm:
            return { 4, Image::BaseType::INT8 };

        case rhi::Format::BGRA8Unorm:
        case rhi::Format::BGRA8UnormSrgb:
        case rhi::Format::BGRX8Unorm:
        case rhi::Format::BGRX8UnormSrgb:
            return { 4, Image::BaseType::UINT8 };

        case rhi::Format::R16Uint:
        case rhi::Format::R16Unorm:
            return { 1, Image::BaseType::UINT16 };
        case rhi::Format::R16Sint:
        case rhi::Format::R16Snorm:
            return { 1, Image::BaseType::INT16 };
        case rhi::Format::R16Float:
            return { 1, Image::BaseType::HALF };

        case rhi::Format::RG16Uint:
        case rhi::Format::RG16Unorm:
            return { 2, Image::BaseType::UINT16 };
        case rhi::Format::RG16Sint:
        case rhi::Format::RG16Snorm:
            return { 2, Image::BaseType::INT16 };
        case rhi::Format::RG16Float:
            return { 2, Image::BaseType::HALF };

        case rhi::Format::RGBA16Uint:
        case rhi::Format::RGBA16Unorm:
            return { 4, Image::BaseType::UINT16 };
        case rhi::Format::RGBA16Sint:
        case rhi::Format::RGBA16Snorm:
            return { 4, Image::BaseType::INT16 };
        case rhi::Format::RGBA16Float:
            return { 4, Image::BaseType::HALF };

        case rhi::Format::R32Uint:
        case rhi::Format::R32Sint:
            return { 0, Image::BaseType::UINT8 };
        case rhi::Format::R32Float:
            return { 1, Image::BaseType::FLOAT };

        case rhi::Format::RG32Uint:
        case rhi::Format::RG32Sint:
            return { 0, Image::BaseType::UINT8 };
        case rhi::Format::RG32Float:
            return { 2, Image::BaseType::FLOAT };

        case rhi::Format::RGB32Uint:
        case rhi::Format::RGB32Sint:
            return { 0, Image::BaseType::UINT8 };
        case rhi::Format::RGB32Float:
            return { 3, Image::BaseType::FLOAT };

        case rhi::Format::RGBA32Uint:
        case rhi::Format::RGBA32Sint:
            return { 0, Image::BaseType::UINT8 };
        case rhi::Format::RGBA32Float:
            return { 4, Image::BaseType::FLOAT };

        case rhi::Format::R64Uint:
        case rhi::Format::R64Sint:
            return { 0, Image::BaseType::UINT8 };

        case rhi::Format::BGRA4Unorm:
        case rhi::Format::B5G6R5Unorm:
        case rhi::Format::BGR5A1Unorm:

        case rhi::Format::RGB9E5Ufloat:
        case rhi::Format::RGB10A2Uint:
        case rhi::Format::RGB10A2Unorm:
        case rhi::Format::R11G11B10Float:
            return { 0, Image::BaseType::UINT8 };

        // Depth/stencil formats
        case rhi::Format::D32Float:
            return { 1, Image::BaseType::FLOAT };
        case rhi::Format::D16Unorm:
            return { 1, Image::BaseType::UINT16 };
        case rhi::Format::D32FloatS8Uint:

        // Compressed formats
        case rhi::Format::BC1Unorm:
        case rhi::Format::BC1UnormSrgb:
        case rhi::Format::BC2Unorm:
        case rhi::Format::BC2UnormSrgb:
        case rhi::Format::BC3Unorm:
        case rhi::Format::BC3UnormSrgb:
        case rhi::Format::BC4Unorm:
        case rhi::Format::BC4Snorm:
        case rhi::Format::BC5Unorm:
        case rhi::Format::BC5Snorm:
        case rhi::Format::BC6HUfloat:
        case rhi::Format::BC6HSfloat:
        case rhi::Format::BC7Unorm:
        case rhi::Format::BC7UnormSrgb:
        case rhi::Format::_Count:
            return { 0, Image::BaseType::UINT8 };
    }

    return { 0, Image::BaseType::UINT8 };
}

rhi::Format getRHIFormat(Image::BaseType baseType, unsigned int channelCount, bool useSrgb)
{
    switch (baseType)
    {
        case Image::BaseType::UINT8:
        {
            switch (channelCount)
            {
                case 4:
                    return useSrgb ? rhi::Format::RGBA8UnormSrgb : rhi::Format::RGBA8Unorm;
                case 2:
                    return rhi::Format::RG8Unorm;
                case 1:
                    return rhi::Format::R8Unorm;
                default:
                    return rhi::Format::Undefined;
            }
        }
        case Image::BaseType::INT8:
        {
            switch (channelCount)
            {
                case 4:
                    return rhi::Format::RGBA8Snorm;
                case 2:
                    return rhi::Format::RG8Snorm;
                case 1:
                    return rhi::Format::R8Snorm;
                default:
                    return rhi::Format::Undefined;
            }
        }
        case Image::BaseType::UINT16:
        {
            switch (channelCount)
            {
                case 4:
                    return rhi::Format::RGBA16Unorm;
                case 2:
                    return rhi::Format::RG16Unorm;
                case 1:
                    return rhi::Format::R16Unorm;
                default:
                    return rhi::Format::Undefined;
            }
        }
        case Image::BaseType::INT16:
        {
            switch (channelCount)
            {
                case 4:
                    return rhi::Format::RGBA16Snorm;
                case 2:
                    return rhi::Format::RG16Snorm;
                case 1:
                    return rhi::Format::R16Snorm;
                default:
                    return rhi::Format::Undefined;
            }
        }
        case Image::BaseType::HALF:
        {
            switch (channelCount)
            {
                case 4:
                    return rhi::Format::RGBA16Float;
                case 2:
                    return rhi::Format::RG16Float;
                case 1:
                    return rhi::Format::R16Float;
                default:
                    return rhi::Format::Undefined;
            }
        }
        case Image::BaseType::FLOAT:
        {
            switch (channelCount)
            {
                case 4:
                    return rhi::Format::RGBA32Float;
                case 3:
                    return rhi::Format::RGB32Float;
                case 2:
                    return rhi::Format::RG32Float;
                case 1:
                    return rhi::Format::R32Float;
                default:
                    return rhi::Format::Undefined;
            }
        }
    }
    return rhi::Format::Undefined;
}

/// Convert a slang type to the corresponding buffer format, used for vertex input layout
rhi::Format getRHIFormat(slang::TypeReflection* slangType)
{
    if (slangType->getKind() == slang::TypeReflection::Kind::Scalar)
    {
        switch (slangType->getScalarType())
        {
            case slang::TypeReflection::ScalarType::Int32:
                return rhi::Format::R32Sint;
            case slang::TypeReflection::ScalarType::UInt32:
                return rhi::Format::R32Uint;
            case slang::TypeReflection::ScalarType::Int64:
                return rhi::Format::R64Sint;
            case slang::TypeReflection::ScalarType::UInt64:
                return rhi::Format::R64Uint;
            case slang::TypeReflection::ScalarType::Float16:
                return rhi::Format::R16Float;
            case slang::TypeReflection::ScalarType::Float32:
                return rhi::Format::R32Float;
            case slang::TypeReflection::ScalarType::Int8:
                return rhi::Format::R8Sint;
            case slang::TypeReflection::ScalarType::UInt8:
                return rhi::Format::R8Uint;
            case slang::TypeReflection::ScalarType::Int16:
                return rhi::Format::R16Sint;
            case slang::TypeReflection::ScalarType::UInt16:
                return rhi::Format::R16Uint;
            default:
                throw ExceptionRenderError("Invalid type in toSlangFormat");
        };
    }

    if (slangType->getKind() == slang::TypeReflection::Kind::Vector)
    {
        if (slangType->getElementType()->getScalarType() != slang::TypeReflection::ScalarType::Float32)
            throw ExceptionRenderError("Invalid type in toSlangFormat");

        switch (slangType->getColumnCount())
        {
            case 1:
                return rhi::Format::R32Float;
            case 2:
                return rhi::Format::RG32Float;
            case 3:
                return rhi::Format::RGB32Float;
            case 4:
                return rhi::Format::RGBA32Float;
            default:
                throw ExceptionRenderError("Invalid type in toSlangFormat");
        }
    }

    throw ExceptionRenderError("Invalid type in toSlangFormat");
}

/// True when the slangType is `SamplerTexture2D`, the Texture2D + SamplerState
bool isSamplerTexture2D(slang::TypeReflection* type)
{
    if (type->getKind() != slang::TypeReflection::Kind::Struct)
        return false;

    static const std::string name = "SamplerTexture2D";
    if (type->getName() != name)
        return false;

    return true;
}

static const std::string STRING_TYPE_NAME = "String";
bool isSlangString(slang::TypeReflection* slangType)
{
    if (slangType->getKind() != slang::TypeReflection::Kind::Struct)
        return false;

    return STRING_TYPE_NAME == slangType->getName();
}

/// Maps Slang scalar-like structs to the corresponding MaterialX type names.
static const std::map<std::string, std::string> scalarLikeSlang2MxTypes = {
    { "SamplerTexture2D", "filename" },
    { "BSDF", "BSDF" },
    { "surfaceshader", "surfaceshader" },
    { "volumeshader", "volumeshader" },
    { "displacementshader", "displacementshader" },
    { "lightshader", "lightshader" }
};

bool isScalarLikeType(slang::TypeReflection* type)
{
    switch (type->getKind())
    {
        case slang::TypeReflection::Kind::None:
            return false;
        case slang::TypeReflection::Kind::Struct:
            return scalarLikeSlang2MxTypes.count(getFullName(type)) > 0;
        case slang::TypeReflection::Kind::Array:
            return false;
        case slang::TypeReflection::Kind::Matrix:
        case slang::TypeReflection::Kind::Vector:
        case slang::TypeReflection::Kind::Scalar:
            return true;
        case slang::TypeReflection::Kind::ConstantBuffer:
        case slang::TypeReflection::Kind::Resource:
        case slang::TypeReflection::Kind::SamplerState:
        case slang::TypeReflection::Kind::TextureBuffer:
        case slang::TypeReflection::Kind::ShaderStorageBuffer:
        case slang::TypeReflection::Kind::ParameterBlock:
        case slang::TypeReflection::Kind::GenericTypeParameter:
        case slang::TypeReflection::Kind::Interface:
        case slang::TypeReflection::Kind::OutputStream:
        case slang::TypeReflection::Kind::Specialized:
        case slang::TypeReflection::Kind::Feedback:
        case slang::TypeReflection::Kind::Pointer:
        case slang::TypeReflection::Kind::DynamicResource:
            return false;
    }

    return false;
}

bool isEqualType(slang::TypeReflection* slangType, TypeDesc mxType)
{
    /// Treats mxType STRING as INT, as Slang does not use String and replaces it with int.
    if (mxType == Type::STRING)
        mxType = Type::INTEGER;

    if (slangType->getKind() == slang::TypeReflection::Kind::Scalar)
    {
        switch (slangType->getScalarType())
        {
            case slang::TypeReflection::ScalarType::Int32:
                return mxType == Type::INTEGER;
            case slang::TypeReflection::ScalarType::Float32:
                return mxType == Type::FLOAT;
            case slang::TypeReflection::ScalarType::Bool:
                return mxType == Type::BOOLEAN;
        };
    }
    else if (slangType->getKind() == slang::TypeReflection::Kind::Vector)
    {
        if (slangType->getElementType()->getKind() == slang::TypeReflection::Kind::Scalar &&
            slangType->getElementType()->getScalarType() == slang::TypeReflection::ScalarType::Float32)
        {
            switch (slangType->getColumnCount())
            {
                case 2:
                    return mxType == Type::VECTOR2;
                case 3:
                    return mxType == Type::VECTOR3 || mxType == Type::COLOR3;
                case 4:
                    return mxType == Type::VECTOR4 || mxType == Type::COLOR4;
            }
        }
    }
    else if (slangType->getKind() == slang::TypeReflection::Kind::Matrix)
    {
        if (slangType->getElementType()->getKind() == slang::TypeReflection::Kind::Scalar &&
            slangType->getElementType()->getScalarType() == slang::TypeReflection::ScalarType::Float32)
        {
            if (slangType->getColumnCount() == 3 && slangType->getRowCount() == 3)
                return mxType == Type::MATRIX33;
            if (slangType->getColumnCount() == 4 && slangType->getRowCount() == 4)
                return mxType == Type::MATRIX44;
        }
    }
    else if (slangType->getKind() == slang::TypeReflection::Kind::Array)
    {
        if (slangType->getElementType()->getKind() == slang::TypeReflection::Kind::Scalar)
        {
            if (slangType->getElementType()->getScalarType() == slang::TypeReflection::ScalarType::Float32)
                return mxType == Type::FLOATARRAY;
            if (slangType->getElementType()->getScalarType() == slang::TypeReflection::ScalarType::Int32)
                return mxType == Type::INTEGERARRAY;
        }
    }

    if (auto it = scalarLikeSlang2MxTypes.find(getFullName(slangType)); it != scalarLikeSlang2MxTypes.end())
    {
        return (it->second == mxType.getName());
    }

    return false;
}

// Returns full name of the slang type, e.g., `vector<4, float>`
string getFullName(slang::TypeReflection* type)
{
    rhi::ComPtr<ISlangBlob> blob;
    type->getFullName(blob.writeRef());
    return string((const char*) blob->getBufferPointer());
}

// Returns byte size of the slang type on the GPU, used for vertex inputs buffer layout
size_t getByteSize(slang::TypeLayoutReflection* typeLayout)
{
    return typeLayout->getSize();
}

namespace
{

template <typename T>
void setTypedArrayValue(const rhi::ShaderCursor& cursor, const ConstValuePtr& value, size_t slangByteSize)
{
    const std::vector<T>& v = value->asA<std::vector<T>>();
    size_t realByteSize = v.size() * sizeof(T);
    if (realByteSize != slangByteSize)
        throw ExceptionRenderError("Mismatch between the expected (" + std::to_string(slangByteSize) + "B) and real (" + std::to_string(realByteSize) + "B) type size.");
    cursor.setData(v.data(), slangByteSize);
}

template <typename T>
void setTypedValue(const rhi::ShaderCursor& cursor, const ConstValuePtr& value, size_t slangByteSize)
{
    if (sizeof(T) != slangByteSize)
        throw ExceptionRenderError("Mismatch between the expected (" + std::to_string(slangByteSize) + "B) and real (" + std::to_string(sizeof(T)) + "B) type size.");
    cursor.setData(value->asA<T>());
}

template <>
void setTypedValue<Matrix33>(const rhi::ShaderCursor& cursor, const ConstValuePtr& value, size_t slangByteSize)
{
    const Matrix33& m33 = value->asA<Matrix33>();
    if (slangByteSize == 36) // destination has 3x float3 layout, e.g., Vulkan
    {
        cursor.setData(m33);
    }
    else if (slangByteSize == 44) // destination has 2x float4 + 1x float3 layout, e.g., D3D12
    {
        Matrix44 m44(
            m33[0][0], m33[0][1], m33[0][2], 0.f,
            m33[1][0], m33[1][1], m33[1][2], 0.f,
            m33[2][0], m33[2][1], m33[2][2], 0.f,
            0.f, 0.f, 0.f, 1.f);
        cursor.setData(&m44, slangByteSize);
    }
    else
    {
        throw ExceptionRenderError("Unsupported matrix size " + std::to_string(slangByteSize) + "B.");
    }
}

template <>
void setTypedValue<bool>(const rhi::ShaderCursor& cursor, const ConstValuePtr& value, size_t slangByteSize)
{
    // On write we convert the bool to an int of appropriate size before writing it.
    if (slangByteSize == 1)
    {
        cursor.setData(int8_t(value->asA<bool>() ? 1 : 0));
    }
    else if (slangByteSize == 2)
    {
        cursor.setData(int16_t(value->asA<bool>() ? 1 : 0));
    }
    else if (slangByteSize == 4)
    {
        cursor.setData(int32_t(value->asA<bool>() ? 1 : 0));
    }
    else
    {
        throw ExceptionRenderError("Unsupported boolean size " + std::to_string(slangByteSize) + "B.");
    }
}

template <>
void setTypedValue<std::vector<float>>(const rhi::ShaderCursor& cursor, const ConstValuePtr& value, size_t slangByteSize)
{
    return setTypedArrayValue<float>(cursor, value, slangByteSize);
}

template <>
void setTypedValue<std::vector<int>>(const rhi::ShaderCursor& cursor, const ConstValuePtr& value, size_t slangByteSize)
{
    return setTypedArrayValue<int>(cursor, value, slangByteSize);
}

} // namespace

void setSlangValue(const rhi::ShaderCursor& cursor, const ConstValuePtr& value, size_t slangByteSize)
{
    if (!value)
        throw ExceptionRenderError("Trying to set variable without an empty value.");

    const std::string& typeString = value->getTypeString();
    if (typeString == Type::FLOAT.getName())
    {
        setTypedValue<float>(cursor, value, slangByteSize);
    }
    else if (typeString == Type::INTEGER.getName())
    {
        setTypedValue<int>(cursor, value, slangByteSize);
    }
    else if (typeString == Type::INTEGERARRAY.getName())
    {
        setTypedValue<std::vector<int>>(cursor, value, slangByteSize);
    }
    else if (typeString == Type::FLOATARRAY.getName())
    {
        setTypedValue<std::vector<float>>(cursor, value, slangByteSize);
    }
    else if (typeString == Type::BOOLEAN.getName())
    {
        setTypedValue<bool>(cursor, value, slangByteSize);
    }
    else if (typeString == Type::COLOR3.getName())
    {
        setTypedValue<Color3>(cursor, value, slangByteSize);
    }
    else if (typeString == Type::COLOR4.getName())
    {
        setTypedValue<Color4>(cursor, value, slangByteSize);
    }
    else if (typeString == Type::VECTOR2.getName())
    {
        setTypedValue<Vector2>(cursor, value, slangByteSize);
    }
    else if (typeString == Type::VECTOR3.getName())
    {
        setTypedValue<Vector3>(cursor, value, slangByteSize);
    }
    else if (typeString == Type::VECTOR4.getName())
    {
        setTypedValue<Vector4>(cursor, value, slangByteSize);
    }
    else if (typeString == Type::MATRIX33.getName())
    {
        setTypedValue<Matrix33>(cursor, value, slangByteSize);
    }
    else if (typeString == Type::MATRIX44.getName())
    {
        setTypedValue<Matrix44>(cursor, value, slangByteSize);
    }
    else
    {
        throw ExceptionRenderError("Unsupported data type `" + typeString + "` when setting uniform value");
    }
}

void clearSlangValue(const rhi::ShaderCursor& cursor, size_t slangByteSize)
{
    static constexpr size_t staticZerodata[4] = { 0, 0, 0, 0 };
    static constexpr size_t staticZerodataByteSize = sizeof(staticZerodata);

    if (slangByteSize > staticZerodataByteSize)
    {
        std::vector<uint8_t> dynamicZerodata(slangByteSize, 0);
        cursor.setData(staticZerodata, slangByteSize);
    }
    else
    {
        cursor.setData(staticZerodata, slangByteSize);
    }
}

MATERIALX_NAMESPACE_END
