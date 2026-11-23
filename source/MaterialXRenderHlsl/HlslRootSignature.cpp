//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslRootSignature.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

void addStageParameters(const HlslStageReflection& stage, HlslShaderVisibility visibility,
                        std::vector<HlslRootParameter>& parameters)
{
    for (const HlslReflectedCbuffer& cb : stage.getCbuffers())
    {
        HlslRootParameter p;
        p.type = HlslRootParameterType::ConstantBuffer;
        p.visibility = visibility;
        p.registerBase = cb.slot;
        p.registerCount = 1;
        p.space = cb.space;
        p.name = cb.name;
        parameters.push_back(p);
    }

    const std::pair<HlslResourceType, HlslRootParameterType> tables[] = {
        { HlslResourceType::Texture, HlslRootParameterType::TextureTable },
        { HlslResourceType::Sampler, HlslRootParameterType::SamplerTable },
    };
    for (const auto& table : tables)
    {
        for (unsigned int space : stage.getSpaces(table.first))
        {
            HlslRootParameter p;
            p.type = table.second;
            p.visibility = visibility;
            p.registerBase = 0;
            p.registerCount = stage.getSlotCount(table.first, space);
            p.space = space;
            if (p.registerCount > 0)
                parameters.push_back(p);
        }
    }
}

} // namespace

HlslRootSignatureDesc HlslRootSignatureDesc::create(const HlslStageReflection& vertex,
                                                    const HlslStageReflection& pixel)
{
    HlslRootSignatureDesc desc;
    addStageParameters(vertex, HlslShaderVisibility::Vertex, desc._parameters);
    addStageParameters(pixel, HlslShaderVisibility::Pixel, desc._parameters);
    return desc;
}

int HlslRootSignatureDesc::findConstantBuffer(HlslShaderVisibility visibility, unsigned int reg, unsigned int space) const
{
    for (std::size_t i = 0; i < _parameters.size(); ++i)
    {
        const HlslRootParameter& p = _parameters[i];
        if (p.type == HlslRootParameterType::ConstantBuffer && p.visibility == visibility &&
            p.registerBase == reg && p.space == space)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int HlslRootSignatureDesc::findTable(HlslRootParameterType type, HlslShaderVisibility visibility, unsigned int space) const
{
    for (std::size_t i = 0; i < _parameters.size(); ++i)
    {
        const HlslRootParameter& p = _parameters[i];
        if (p.type == type && p.visibility == visibility && p.space == space)
            return static_cast<int>(i);
    }
    return -1;
}

unsigned int HlslRootSignatureDesc::getRootCost() const
{
    unsigned int cost = 0;
    for (const HlslRootParameter& p : _parameters)
        cost += (p.type == HlslRootParameterType::ConstantBuffer) ? 2u : 1u;
    return cost;
}

MATERIALX_NAMESPACE_END
