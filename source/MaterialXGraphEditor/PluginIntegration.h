//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_PLUGIN_INTEGRATION_H
#define MATERIALX_PLUGIN_INTEGRATION_H

#include <string>

/// Load Python-based plugins from the specified directory.
/// This function initializes the Python interpreter if needed and
/// discovers Python plugins that inherit from Plugin interface.
/// @param plugin_dir Path to directory containing Python plugin files
void load_python_plugins(const std::string& plugin_dir);

#endif // MATERIALX_PLUGIN_INTEGRATION_H
