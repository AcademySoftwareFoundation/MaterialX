//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_PLUGIN_INTEGRATION_H
#define MATERIALX_PLUGIN_INTEGRATION_H

#include <MaterialXCore/Util.h>
#include <MaterialXFormat/File.h>
#include <MaterialXCore/Document.h>
#include <MaterialXRender/PluginManager.h>

namespace mx = MaterialX;

class PluginIntegration;
using PluginIntegrationPtr = std::shared_ptr<PluginIntegration>;

class PluginIntegration
{
  public:
    static PluginIntegrationPtr create()
    {
        return std::make_shared<PluginIntegration>();
    }
    PluginIntegration();
    virtual ~PluginIntegration();

    bool initialize();

    void loadPythonPlugins();
    mx::StringVec getPluginList() const
    {
        return _pluginList;
    }
    mx::DocumentPtr loadDocument(const std::string& pluginName, const mx::FilePath& path) const;    std::vector<mx::DocumentLoaderPluginPtr> getDocumentLoaderPlugins(const std::string& pluginName) const;
    bool saveDocument(const std::string& pluginName, mx::DocumentPtr doc, const mx::FilePath& path) const;

    mx::PluginManagerPtr getPluginManager() const;

  protected:      
    mx::StringVec _pluginList;

  private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

#endif // MATERIALX_PLUGIN_INTEGRATION_H

