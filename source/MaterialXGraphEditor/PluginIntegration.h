//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GRAPH_EDITOR_PLUGIN_INTEGRATION_H
#define MATERIALX_GRAPH_EDITOR_PLUGIN_INTEGRATION_H

#include <MaterialXGraphEditor/Graph.h>
#include <MaterialXRender/PluginManager.h>

#include <string>
#include <vector>
#include <functional>

namespace mx = MaterialX;

/// Integration class for MaterialX plugins in MaterialXGraphEditor
class GraphEditorPluginIntegration
{
  public:
    GraphEditorPluginIntegration();
    ~GraphEditorPluginIntegration() = default;

    /// Initialize the plugin system
    void initialize();

    /// Register a document loader callback for the graph editor
    void setDocumentLoader(std::function<mx::DocumentPtr(const mx::FilePath&)> loader);

    /// Register a document saver callback for the graph editor  
    void setDocumentSaver(std::function<bool(mx::ConstDocumentPtr, const mx::FilePath&)> saver);

    /// Try to load a document using plugins
    /// Returns nullptr if no suitable plugin found
    mx::DocumentPtr loadDocumentWithPlugins(const mx::FilePath& filename);

    /// Try to save a document using plugins
    /// Returns false if no suitable plugin found or export failed
    bool saveDocumentWithPlugins(mx::ConstDocumentPtr document, const mx::FilePath& filename);

    /// Get import file dialog filters compatible with the file dialog system
    std::string getImportFileFilters();

    /// Get export file dialog filters compatible with the file dialog system
    std::string getExportFileFilters();

    /// Check if a file can be imported using plugins
    bool canImportFile(const mx::FilePath& filename);

    /// Check if a file can be exported using plugins
    bool canExportFile(const mx::FilePath& filename);

    /// Get list of available plugins for UI display
    std::vector<mx::PluginInfo> getAvailablePlugins();

    /// Get the plugin manager instance
    mx::PluginManager& getPluginManager() { return mx::PluginManager::getInstance(); }

  private:
    std::function<mx::DocumentPtr(const mx::FilePath&)> _documentLoader;
    std::function<bool(mx::ConstDocumentPtr, const mx::FilePath&)> _documentSaver;
    bool _initialized;
};

#endif
