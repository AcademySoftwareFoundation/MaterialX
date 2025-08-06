//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_RENDER_PLUGINMANAGER_H
#define MATERIALX_RENDER_PLUGINMANAGER_H

/// @file
/// Plugin manager for MaterialX handler registration and discovery

#include <MaterialXRender/Export.h>
#include <MaterialXCore/Document.h>
#include <MaterialXRender/DocumentHandler.h>

#include <memory>
#include <string>
#include <vector>
#include <functional>

MATERIALX_NAMESPACE_BEGIN

/// Plugin manager class that handles registration and discovery of various handler types
class MX_RENDER_API PluginManager
{
  public:
    /// Get the singleton instance of the plugin manager
    static PluginManager& getInstance();

    /// Register a document loader
    /// @param loader Shared pointer to the document loader
    /// @return True if registration was successful
    bool registerDocumentLoader(DocumentLoaderPtr loader);

    /// Unregister a document loader by identifier
    /// @param identifier The loader identifier
    /// @return True if unregistration was successful
    bool unregisterDocumentLoader(const std::string& identifier);

    /// Import a document using the best available handler
    /// @param uri The uri to import
    /// @return A MaterialX document or nullptr on failure
    DocumentPtr importDocument(const std::string& uri);

    /// Export a document using the best available handler
    /// @param document The document to export
    /// @param uri The uri to export to
    /// @return True on success, false on failure
    bool exportDocument(ConstDocumentPtr document, const std::string& uri);

    /// Set callback for handler registration events
    /// @param callback Function to call when handlers are registered/unregistered
    void setRegistrationCallback(std::function<void(const std::string&, bool)> callback)
    {
        _registrationCallback = callback;
    }

  private:
    PluginManager() = default;
    ~PluginManager() = default;
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;

    DocumentHandlerPtr _documentHandler;
    std::function<void(const std::string&, bool)> _registrationCallback;
};

MATERIALX_NAMESPACE_END

#endif
