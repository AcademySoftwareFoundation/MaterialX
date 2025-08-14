//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_PLUGIN_INTEGRATION_H
#define MATERIALX_PLUGIN_INTEGRATION_H

#include <MaterialXCore/Util.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Environ.h>
#include <MaterialXCore/Document.h>

#include <pybind11/embed.h>

namespace mx = MaterialX;
namespace py = pybind11;

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
    virtual ~PluginIntegration() {};

    void loadPythonPlugins();
    mx::StringVec getPluginList() const
    {
        return _pluginList;
    }
    mx::DocumentPtr loadDocument(const std::string& pluginName, const mx::FilePath& path) const;
    bool saveDocument(const std::string& pluginName, mx::DocumentPtr doc, const mx::FilePath& path) const;

  protected:
      
    mx::StringVec _pluginList;

    std::unique_ptr<py::scoped_interpreter> _pyInterpreter;
    py::object _pymxModule;
    py::object _mypluginsModule;
};

#endif // MATERIALX_PLUGIN_INTEGRATION_H

