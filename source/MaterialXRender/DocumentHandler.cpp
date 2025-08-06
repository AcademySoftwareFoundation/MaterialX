//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRender/DocumentHandler.h>
#include <MaterialXCore/Util.h>

MATERIALX_NAMESPACE_BEGIN

/// Add loader to the handler
bool DocumentHandler::registerLoader(DocumentLoaderPtr loader)
{
    if (!loader)
    {
        return false;
    }

    if (getLoader(loader->getIdentifier()))
    {
        return false;
    }

    _loaders.push_back(loader);
    return true;
}

DocumentLoaderPtr DocumentHandler::getLoader(const string& identifier)
{
    auto it = std::find_if(_loaders.begin(), _loaders.end(),
        [&identifier](const DocumentLoaderPtr& loader) {
            return loader->getIdentifier() == identifier;
        });
    if (it != _loaders.end())
    {
        return *it;
    }
    return nullptr;
}

bool DocumentHandler::unregisterLoader(const string& identifier)
{
    auto it = std::find_if(_loaders.begin(), _loaders.end(),
        [&identifier](const DocumentLoaderPtr& loader) {
            return loader->getIdentifier() == identifier;
        });

    if (it != _loaders.end())
    {
        _loaders.erase(it);        
        return true;
    }
    return false;    
}

DocumentPtr DocumentHandler::importDocument(const string& uri)
{
    for (auto loader : _loaders)
    {
        DocumentPtr doc = loader->importDocument(uri);
        if (doc)
        {
            return doc; 
        }
    }
    return nullptr;
}

bool DocumentHandler::exportDocument(ConstDocumentPtr doc, const string& uri)
{
    for (auto loader : _loaders)
    {
        if (loader->exportDocument(doc, uri))
        {
            return true; 
        }
    }
    return false;
}

/// Get a list of extensions supported by the handler.
StringSet DocumentHandler::supportedExtensions()
{
    StringSet extensions;
    for (auto loader : _loaders)
    {
        StringSet loaderExtensions = loader->supportedExtensions();
        extensions.insert(loaderExtensions.begin(), loaderExtensions.end());
    }
    return extensions;
}


MATERIALX_NAMESPACE_END
