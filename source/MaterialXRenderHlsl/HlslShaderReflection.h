//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLSHADERREFLECTION_H
#define MATERIALX_HLSLSHADERREFLECTION_H

/// @file
/// Backend-neutral reflection data for a compiled HLSL stage.

#include <MaterialXRenderHlsl/Export.h>

#include <MaterialXCore/Library.h>

#include <cstddef>
#include <string>
#include <vector>

MATERIALX_NAMESPACE_BEGIN

/// Class of a HLSL resource as reported by D3D shader reflection. Only the
/// classes we care about for graphics shaders are exposed.
enum class HlslResourceType
{
    CBuffer,        ///< Constant buffer (b#)
    Texture,        ///< Shader resource view (t#)
    Sampler,        ///< Sampler (s#)
    Other           ///< Any other binding D3D reports (uavs, structured buffers, ...)
};

/// One reflected resource binding from a compiled HLSL stage.
struct HlslResourceBinding
{
    std::string name;        ///< Binding name as seen in the source.
    HlslResourceType type = HlslResourceType::Other;
    unsigned int slot = 0;   ///< Register slot index (b#, t#, s# depending on type).
    unsigned int space = 0;  ///< Register space (D3D12 root signatures).
    unsigned int count = 1;  ///< Array count, or 1 for scalar bindings.
};

/// One member of a struct type declared in a constant buffer.
struct HlslReflectedMember
{
    std::string name;
    unsigned int offset = 0; ///< Byte offset relative to the start of the struct.
};

/// One variable declared in a constant buffer.
struct HlslReflectedVariable
{
    std::string name;
    unsigned int offset = 0;   ///< Byte offset within the constant buffer.
    unsigned int size = 0;     ///< Total byte size, every array element included.
    unsigned int elements = 0; ///< Array element count, 0 for a non-array variable.
    std::vector<HlslReflectedMember> members; ///< Struct members, empty for non-struct types.

    /// Return the byte stride between consecutive array elements. HLSL
    /// starts every constant buffer array element on a 16-byte boundary
    /// but does not pad the last one, so the stride is the per-element
    /// size rounded up to 16 bytes.
    unsigned int getElementStride() const;
};

/// One constant buffer declared by a stage.
struct HlslReflectedCbuffer
{
    std::string name;
    unsigned int slot = 0;  ///< b# register.
    unsigned int space = 0; ///< Register space (always 0 for DXBC).
    unsigned int size = 0;  ///< Byte size as reported by reflection.
    std::vector<HlslReflectedVariable> variables;
};

/// Component type of a shader input parameter.
enum class HlslComponentType
{
    Float,
    SInt,
    UInt
};

/// One input parameter of a stage's input signature. System values such
/// as SV_Position are not reported.
struct HlslInputParameter
{
    std::string semanticName;
    unsigned int semanticIndex = 0;
    unsigned int componentCount = 0; ///< Declared components, 1 to 4.
    HlslComponentType componentType = HlslComponentType::Float;
};

/// Location of a value inside one of a stage's constant buffers.
struct HlslUniformLocation
{
    std::size_t cbufferIndex = 0; ///< Index into HlslStageReflection::getCbuffers().
    std::size_t offset = 0;       ///< Byte offset inside that constant buffer.
};

/// @class HlslStageReflection
/// Resource bindings and constant buffer layouts of one compiled stage,
/// captured once from D3D shader reflection so that callers can look up
/// uniform offsets without holding any D3D object. Filled by
/// HlslProgram::reflectStage for both DXIL (DXC) and DXBC (FXC) bytecode.
class MX_RENDERHLSL_API HlslStageReflection
{
  public:
    HlslStageReflection() = default;

    /// Return true if the bytecode was reflected successfully.
    bool isValid() const { return _valid; }

    /// Every bound resource (constant buffers, textures, samplers).
    const std::vector<HlslResourceBinding>& getBindings() const { return _bindings; }

    /// Every constant buffer with its variable layout.
    const std::vector<HlslReflectedCbuffer>& getCbuffers() const { return _cbuffers; }

    /// Input signature of the stage, in declaration order.
    const std::vector<HlslInputParameter>& getInputs() const { return _inputs; }

    /// Return the index of the named constant buffer, or -1 if absent.
    int findCbuffer(const std::string& name) const;

    /// Locate a top-level constant buffer variable by name, searching every
    /// constant buffer of the stage. Returns false if no buffer declares it.
    bool findVariable(const std::string& name, HlslUniformLocation& location) const;

    /// Locate the member `<arrayName>[index].<memberName>` of a struct-array
    /// variable. D3D reflection only names the array variable itself, so the
    /// location is composed from the array offset, element stride, and
    /// member offset. Returns false if any part is missing or out of range.
    bool findArrayMember(const std::string& arrayName, std::size_t index,
                         const std::string& memberName, HlslUniformLocation& location) const;

    /// Return one past the highest register slot used by resources of the
    /// given type in the given register space, or 0 if there are none.
    unsigned int getSlotCount(HlslResourceType type, unsigned int space = 0) const;

    /// Return the sorted list of register spaces used by resources of the
    /// given type.
    std::vector<unsigned int> getSpaces(HlslResourceType type) const;

  protected:
    friend class HlslProgram;

    bool _valid = false;
    std::vector<HlslResourceBinding> _bindings;
    std::vector<HlslReflectedCbuffer> _cbuffers;
    std::vector<HlslInputParameter> _inputs;
};

MATERIALX_NAMESPACE_END

#endif
