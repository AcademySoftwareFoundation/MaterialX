//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RENDER_UTIL_H
#define MATERIALX_RENDER_UTIL_H

/// @file
/// Rendering utility methods

#include <MaterialXCore/Document.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Util.h>

#include <map>

namespace MaterialX
{
    /// @name Shader utilities
    /// @{

    /// Create a shader for a given element
    ShaderPtr createShader(const string& shaderName, GenContext& context, ElementPtr elem);

    /// Create a shader with a constant color output for a given element
    ShaderPtr createConstantShader(GenContext& context,
                                   DocumentPtr stdLib,
                                   const string& shaderName,
                                   const Color3& color);

    /// @}
    /// @name User interface utilities
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
    };

    /// Get the UI properties for a given nodedef element.
    /// Returns the number of properties found.
    unsigned int getUIProperties(const ValueElementPtr nodeDefElement, UIProperties& uiProperties);

    /// Get the UI properties for a given element path. If the path is to a node, a target
    /// identifier can be provided.
    /// Returns the number of properties found.
    unsigned int getUIProperties(const string& path, DocumentPtr doc, const string& target, UIProperties& uiProperties);

    /// Interface for holding the UI properties associated shader port
    struct UIPropertyItem
    {
        std::string label;
        ShaderPort* variable = nullptr;
        UIProperties ui;
    };

    /// A grouping of property items by name
    using UIPropertyGroup = std::multimap<string, UIPropertyItem>;

    /// Utility to group UI properties items based on ELement group name.
    /// Returns a list of named and unnamed groups.
    void createUIPropertyGroups(const VariableBlock& block, DocumentPtr contentDocument, TypedElementPtr materialElement,
                                const string& pathSeparator, UIPropertyGroup& groups,
                                UIPropertyGroup& unnamedGroups);

    /// @}

} // namespace MaterialX

#endif