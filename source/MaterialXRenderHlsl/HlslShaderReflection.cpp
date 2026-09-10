//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslShaderReflection.h>

#include <algorithm>

MATERIALX_NAMESPACE_BEGIN

unsigned int HlslReflectedVariable::getElementStride() const
{
    if (elements == 0)
        return size;

    // size = (elements - 1) * stride + elementSize, with elementSize in
    // (stride - 16, stride]. Dividing by the element count therefore lands
    // in (stride - 16, stride], and rounding up to 16 recovers the stride.
    const unsigned int perElement = (size + elements - 1) / elements;
    return (perElement + 15u) & ~15u;
}

int HlslStageReflection::findCbuffer(const std::string& name) const
{
    for (std::size_t i = 0; i < _cbuffers.size(); ++i)
    {
        if (_cbuffers[i].name == name)
            return static_cast<int>(i);
    }
    return -1;
}

bool HlslStageReflection::findVariable(const std::string& name, HlslUniformLocation& location) const
{
    for (std::size_t i = 0; i < _cbuffers.size(); ++i)
    {
        for (const HlslReflectedVariable& var : _cbuffers[i].variables)
        {
            if (var.name == name)
            {
                location.cbufferIndex = i;
                location.offset = var.offset;
                return true;
            }
        }
    }
    return false;
}

bool HlslStageReflection::findArrayMember(const std::string& arrayName, std::size_t index,
                                          const std::string& memberName, HlslUniformLocation& location) const
{
    for (std::size_t i = 0; i < _cbuffers.size(); ++i)
    {
        for (const HlslReflectedVariable& var : _cbuffers[i].variables)
        {
            if (var.name != arrayName || var.elements == 0 || index >= var.elements)
                continue;
            for (const HlslReflectedMember& member : var.members)
            {
                if (member.name == memberName)
                {
                    location.cbufferIndex = i;
                    location.offset = var.offset + index * var.getElementStride() + member.offset;
                    return true;
                }
            }
            return false;
        }
    }
    return false;
}

unsigned int HlslStageReflection::getSlotCount(HlslResourceType type, unsigned int space) const
{
    unsigned int count = 0;
    for (const HlslResourceBinding& b : _bindings)
    {
        if (b.type == type && b.space == space)
            count = std::max(count, b.slot + b.count);
    }
    return count;
}

std::vector<unsigned int> HlslStageReflection::getSpaces(HlslResourceType type) const
{
    std::vector<unsigned int> spaces;
    for (const HlslResourceBinding& b : _bindings)
    {
        if (b.type == type && std::find(spaces.begin(), spaces.end(), b.space) == spaces.end())
            spaces.push_back(b.space);
    }
    std::sort(spaces.begin(), spaces.end());
    return spaces;
}

MATERIALX_NAMESPACE_END
