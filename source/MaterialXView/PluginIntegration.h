//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_VIEW_PLUGIN_INTEGRATION_H
#define MATERIALX_VIEW_PLUGIN_INTEGRATION_H

#include <MaterialXView/Editor.h>
#include <MaterialXRender/PluginManager.h>

#include <string>
#include <vector>

namespace mx = MaterialX;

/// Integration class for MaterialX plugins in MaterialXView
class PluginIntegration
{
  public:
    PluginIntegration();
    ~PluginIntegration() = default;    /// Initialize the plugin system
    void initialize();

    /// Shutdown the plugin system and clean up callbacks
    void shutdown();

    /// Register a document loader callback for the viewer
    /// This will be called when the viewer needs to load a document
    void setDocumentLoader(std::function<mx::DocumentPtr(const mx::FilePath&)> loader);

    /// Try to load a document using plugins
    /// Returns nullptr if no suitable plugin found
    mx::DocumentPtr loadDocumentWithPlugins(const mx::FilePath& filename);

    /// Try to save a document using plugins
    /// Returns false if no suitable plugin found or export failed
    bool saveDocumentWithPlugins(mx::ConstDocumentPtr document, const mx::FilePath& filename);

    /// Get supported import file filters for file dialogs
    /// Returns a vector of pairs (description, pattern)
    std::vector<std::pair<std::string, std::string>> getImportFileFilters();

    /// Get supported export file filters for file dialogs
    /// Returns a vector of pairs (description, pattern)
    std::vector<std::pair<std::string, std::string>> getExportFileFilters();

    /// Check if a file can be imported using plugins
    bool canImportFile(const mx::FilePath& filename);

    /// Check if a file can be exported using plugins
    bool canExportFile(const mx::FilePath& filename);

    /// Get the plugin manager instance
    mx::PluginManager& getPluginManager() { return mx::PluginManager::getInstance(); }

  private:
    std::function<mx::DocumentPtr(const mx::FilePath&)> _documentLoader;
    bool _initialized;
};

#endif
