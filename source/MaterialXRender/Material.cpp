//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRender/Material.h>
#include <MaterialXFormat/XmlIo.h>

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

bool Material::generateEnvironmentShader(GenContext& context,
                                         const FilePath& filename,
                                         DocumentPtr stdLib,
                                         const FilePath& imagePath)
{
    // Read in the environment nodegraph.
    DocumentPtr doc = createDocument();
    doc->importLibrary(stdLib);
    DocumentPtr envDoc = createDocument();
    readFromXmlFile(envDoc, filename);
    doc->importLibrary(envDoc);

    NodeGraphPtr envGraph = doc->getNodeGraph("environmentDraw");
    if (!envGraph)
    {
        return false;
    }
    NodePtr image = envGraph->getNode("envImage");
    if (!image)
    {
        return false;
    }
    image->setInputValue("file", imagePath.asString(), FILENAME_TYPE_STRING);
    OutputPtr output = envGraph->getOutput("out");
    if (!output)
    {
        return false;
    }

    // Create the shader.
    std::string shaderName = "__ENV_SHADER__";
    _hwShader = createShader(shaderName, context, output);
    if (!_hwShader)
    {
        return false;
    }
    return generateShader(_hwShader);
}

MATERIALX_NAMESPACE_END
