//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRender/ShaderMaterial.h>
#include <MaterialXFormat/XmlIo.h>

MATERIALX_NAMESPACE_BEGIN

ShaderMaterial::ShaderMaterial() {}
ShaderMaterial::~ShaderMaterial() {}

std::map<ShaderMaterialDefinition, std::weak_ptr<ShaderMaterialState>> ShaderMaterial::_sStateCache;

void ShaderMaterial::setDocument(DocumentPtr doc)
{
    _def.doc = doc;
    // Clear the state, as the definition for this material has changed.
    _pState.reset();
}

DocumentPtr ShaderMaterial::getDocument() const
{
    return _def.doc;
}

void ShaderMaterial::setElement(TypedElementPtr val)
{
    _def.elem = val;
    // Clear the state, as the definition for this material has changed.
    _pState.reset();
}

TypedElementPtr ShaderMaterial::getElement() const
{
    return _def.elem;
}

void ShaderMaterial::setMaterialNode(NodePtr node)
{
    _def.materialNode = node;
    // Clear the state, as the definition for this material has changed.
    _pState.reset();
}

NodePtr ShaderMaterial::getMaterialNode() const
{
    return _def.materialNode;
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
    // If there not state currently associated with this material, find or create one.
    if (!_pState) {

        // See if there is state already cached for the current definition.
        auto iter = _sStateCache.find(_def);
        if (iter != _sStateCache.end()) {
            _pState = iter->second.lock();
            // Cache stores weak pointers that may have been invaliding
            if (!_pState)
                _sStateCache.erase(_def);
        }
        // Create a new state if one was not found in the cache.
        if (!_pState) {
            _pState = createState();
            _sStateCache[_def] = _pState;
            // Only create generate shader if there is no existing state.
            // TODO: This is proof of concept code, this behavior is probably not what we want in production.
            if (!_pState->generateShader(context))
            {
                _pState.reset();
                return false;
            }
        }
    }
    // Do nothing if we already have valid state.
    // TODO: This is proof of concept code, this behavior is probably not what we want in production.
    return true;
}

bool ShaderMaterial::generateShader(ShaderPtr hwShader)
{
    // Generate new state without looking in cache, as we are ignoring the def in this case.
    _pState = createState();

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
