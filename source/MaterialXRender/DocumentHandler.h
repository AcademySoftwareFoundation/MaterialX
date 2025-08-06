//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_RENDER_DocumentLoader_H
#define MATERIALX_RENDER_DocumentLoader_H

/// @file
/// Document handler interfaces

#include <MaterialXRender/Export.h>
#include <MaterialXCore/Document.h>

#include <memory>
#include <vector>
#include <map>

MATERIALX_NAMESPACE_BEGIN

class DocumentLoader;
class DocumentHandler;

/// Shared pointer to an DocumentHandler
using DocumentHandlerPtr = std::shared_ptr<DocumentHandler>;

/// Shared pointer to a loader
using DocumentLoaderPtr = std::shared_ptr<DocumentLoader>;

/// Map from strings to vectors of loaders
using DocumentLoaderVec = std::vector<DocumentLoaderPtr>;

/// @class DocumentLoader
/// Base class for loaders that can import and/or export MaterialX documents
class MX_RENDER_API DocumentLoader
{
  public:

    DocumentLoader(const string& identifier, const string& name, const string& description) :
        _identifier(identifier),
        _name(name), 
        _description(description)
    {
    }
    virtual ~DocumentLoader() = default;

    /// Get handler identifier
    const string& getIdentifier() const 
    { 
        return _identifier; 
    }

    // Get name
    const string& getName() 
    {
        return _name;
    }

    // Get description
    const string& getDescription()
    {
        return _description;
    }

    /// Returns a list of supported extensions
    /// @return List of support extensions
    virtual StringSet supportedExtensions() const
    {
        return StringSet();
    }

    /// Import a document
    /// Default implementation returns nullptr. Derived classes should override
    /// @param uri The uri to import
    /// @return A MaterialX document or nullptr on failure
    virtual DocumentPtr importDocument(const string&)
    {
        return nullptr;
    }

    /// Export a document
    /// Default implementation returns false. Derived classes should override
    /// @param document The document to export
    /// @param uri The output uri
    /// @return True on success, false on failure
    virtual bool exportDocument(ConstDocumentPtr, const string&)
    {
        return false;
    }

  protected:
    string _identifier;
    string _name;
    string _description;
};

/// @class DocumentHandler
/// Document handler class. 
class MX_RENDER_API DocumentHandler
{
  public:    
    static DocumentHandlerPtr create();
    virtual ~DocumentHandler() { }

    /// Add loader to the handler
    bool registerLoader(DocumentLoaderPtr loader);

    // Remove loader from handler
    bool unregisterLoader(const string& identifier);

    // Get loader based on identifier
    DocumentLoaderPtr getLoader(const string& identifier);    

    DocumentPtr importDocument(const string&);

    bool exportDocument(ConstDocumentPtr, const string&);

    /// Get a list of extensions supported by the handler.
    StringSet supportedExtensions();

protected:
    DocumentLoaderVec _loaders;
};
MATERIALX_NAMESPACE_END

#endif
