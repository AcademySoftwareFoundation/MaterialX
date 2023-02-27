//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRender/Material.h>

MATERIALX_NAMESPACE_BEGIN

Material::Material() : _hasTransparency(false) {}
Material::~Material() {}

void Material::setDocument(DocumentPtr doc)
{
    _doc = doc;
}

DocumentPtr Material::getDocument() const
{
    return _doc;
}

void Material::setElement(TypedElementPtr val)
{
    _elem = val;
}

TypedElementPtr Material::getElement() const
{
    return _elem;
}

void Material::setMaterialNode(NodePtr node)
{
    _materialNode = node;
}

NodePtr Material::getMaterialNode() const
{
    return _materialNode;
}

void Material::setUdim(const std::string& val)
{
    _udim = val;
}

const std::string& Material::getUdim()
{
    return _udim;
}

ShaderPtr Material::getShader() const
{
    return _hwShader;
}

bool Material::hasTransparency() const
{
    return _hasTransparency;
}

MATERIALX_NAMESPACE_END