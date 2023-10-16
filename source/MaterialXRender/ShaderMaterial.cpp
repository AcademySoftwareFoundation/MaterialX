//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRender/ShaderMaterial.h>
#include <MaterialXFormat/XmlIo.h>

MATERIALX_NAMESPACE_BEGIN

ShaderMaterial::ShaderMaterial() {}
ShaderMaterial::~ShaderMaterial() {}

std::map<MaterialDefinition, std::weak_ptr<MaterialDefinitionState>> ShaderMaterial::_sDefinitions;

void ShaderMaterial::setDocument(DocumentPtr doc)
{
    _def._doc = doc;
    _pState.reset();
}

DocumentPtr ShaderMaterial::getDocument() const
{
    return _def._doc;
}

void ShaderMaterial::setElement(TypedElementPtr val)
{
    _def._elem = val;
    _pState.reset();
}

TypedElementPtr ShaderMaterial::getElement() const
{
    return _def._elem;
}

void ShaderMaterial::setMaterialNode(NodePtr node)
{
    _def._materialNode = node;
    _pState.reset();
}

NodePtr ShaderMaterial::getMaterialNode() const
{
    return _def._materialNode;
}

void ShaderMaterial::setOverride(OverridePtr override)
{
    _override = override;
}

OverridePtr ShaderMaterial::getOverride() const
{
    return _override;
}
void ShaderMaterial::setUdim(const std::string& val)
{
    _udim = val;
}

const std::string& ShaderMaterial::getUdim()
{
    return _udim;
}

ShaderPtr ShaderMaterial::getShader() const
{
    return _pState?_pState->getShader():nullptr;
}

bool ShaderMaterial::hasTransparency() const
{
    return _pState ? _pState->hasTransparency() : false;
}

bool ShaderMaterial::generateShader(GenContext& context)
{
    if (!_pState) {
        auto iter = _sDefinitions.find(_def);
        if (iter != _sDefinitions.end()) {
            _pState = iter->second.lock();
            if (!_pState)
                _sDefinitions.erase(_def);
        }
        if (!_pState) {
            _pState = createDefinitionState();
            _sDefinitions[_def] = _pState;
            if (!_pState->generateShader(context))
            {
                _pState.reset();
                return false;
            }
        }
    }
    return true;
}

bool ShaderMaterial::generateShader(ShaderPtr hwShader)
{
    // Generate new state.
    _pState = createDefinitionState();

    return _pState->generateShader(hwShader);
}

bool ShaderMaterial::generateEnvironmentShader(GenContext& context,
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
    ShaderPtr hwShader = createShader(shaderName, context, output);
    if (hwShader)
    {
        return false;
    }
    return generateShader(hwShader);
}

MATERIALX_NAMESPACE_END
