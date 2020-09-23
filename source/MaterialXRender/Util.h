//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RENDER_UTIL_H
#define MATERIALX_RENDER_UTIL_H

/// @file
/// Rendering utility methods

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <map>

namespace MaterialX
{

/// @name Shader Utilities
/// @{

/// Create a shader for a given element.
ShaderPtr createShader(const string& shaderName, GenContext& context, ElementPtr elem);

/// Create a shader with a constant color output, using the given standard libraries
/// for code generation.
ShaderPtr createConstantShader(GenContext& context,
                               DocumentPtr stdLib,
                               const string& shaderName,
                               const Color3& color);

/// Create a shader with depth value output, using the given standard libraries
/// for code generation.
ShaderPtr createDepthShader(GenContext& context,
                            DocumentPtr stdLib,
                            const string& shaderName);

/// Create a shader that generates a look-up table for directional albedo, using
/// the given standard libraries for code generation.
ShaderPtr createAlbedoTableShader(GenContext& context,
                                  DocumentPtr stdLib,
                                  const string& shaderName);

/// Create a blur shader, using the given standard libraries for code generation.
ShaderPtr createBlurShader(GenContext& context,
                           DocumentPtr stdLib,
                           const string& shaderName,
                           const string& filterType,
                           float filterSize);

/// @}
/// @name User Interface Utilities
/// @{

/// Set of possible UI properties for an element
struct UIProperties
{
    /// UI name
    string uiName;

    /// UI folder
    string uiFolder;

    /// Enumeration
    StringVec enumeration;

    /// Enumeration Values
    vector<ValuePtr> enumerationValues;

    /// UI minimum value
    ValuePtr uiMin;

    /// UI maximum value
    ValuePtr uiMax;

    /// UI soft minimum value
    ValuePtr uiSoftMin;

    /// UI soft maximum value
    ValuePtr uiSoftMax;

    /// UI step value
    ValuePtr uiStep;

    /// UI advanced element
    bool uiAdvanced = false;
};

/// Get the UI properties for a given nodedef element.
/// Returns the number of properties found.
unsigned int getUIProperties(ConstValueElementPtr nodeDefElement, UIProperties& uiProperties);

/// Get the UI properties for a given element path. If the path is to a node, a target
/// identifier can be provided.
/// Returns the number of properties found.
unsigned int getUIProperties(const string& path, DocumentPtr doc, const string& target, UIProperties& uiProperties);

/// Interface for holding the UI properties associated shader port
struct UIPropertyItem
{
    string label;
    ValuePtr value;
    ShaderPort* variable = nullptr;
    UIProperties ui;
};

/// A grouping of property items by name
using UIPropertyGroup = std::multimap<string, UIPropertyItem>;

/// Utility to group UI properties items based on Element group name from an element.
/// Returns a list of named and unnamed groups.
void createUIPropertyGroups(ElementPtr uniformElement, DocumentPtr contentDocument, TypedElementPtr materialElement,
                            const string& pathSeparator, UIPropertyGroup& groups,
                            UIPropertyGroup& unnamedGroups, ShaderPort* uniform = nullptr);

/// Utility to group UI properties items based on Element group name from a VariableBlock.
/// Returns a list of named and unnamed groups.
void createUIPropertyGroups(const VariableBlock& block, DocumentPtr contentDocument, TypedElementPtr materialElement,
                            const string& pathSeparator, UIPropertyGroup& groups,
                            UIPropertyGroup& unnamedGroups, bool addFromDefinition);

/// @}

} // namespace MaterialX

#endif
