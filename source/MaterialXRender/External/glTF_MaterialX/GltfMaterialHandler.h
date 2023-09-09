/*

Copyright 2022 - 2023 Bernard Kwok

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifndef MATERIALX_CGLTF_MaterialHandler_H
#define MATERIALX_CGLTF_MaterialHandler_H

// This line added to original source code to place this into the MaterialX Render module.
#define MX_GLTF_API MX_RENDER_API

/// @file 
/// GLTF material loader using the Cgltf library

#include <MaterialXRender/Export.h>
#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>

MATERIALX_NAMESPACE_BEGIN

class MaterialHandler;
class GltfMaterialHandler;

/// Shared pointer to a GltfMaterialHandler
using MaterialHandlerPtr = std::shared_ptr<class MaterialHandler>;
using GltfMaterialHandlerPtr = std::shared_ptr<class GltfMaterialHandler>;

/// @class MaterialHandler
/// Wrapper for handler to convert materials to / from MaterialX
class MX_GLTF_API MaterialHandler
{
  public:
    MaterialHandler() 
        : _generateFullDefinitions(false)
        , _generateAssignments(false)
        , _generateNodeDefs(false)
      {}

    virtual ~MaterialHandler() = default;

    /// Load materials from a given file
    virtual bool load(const FilePath& filePath, StringVec& log) = 0;

    /// Save materials to a given file
    virtual bool save(const FilePath& filePath, StringVec& log) = 0;

    /// Return file extensions supported
    const StringSet& extensionsSupported() const
    {
        return _extensions;
    }

    /// <summary>
    ///     Translate shaders in the document to appropriate shading model(s)
    ///     for export. Derived classes can override this method
    ///     to perform actions such as shader translation and baking.
    /// </summary>
    /// <param name="doc">Document to modify</param>
    /// <param name="log">Error log</param>
    virtual void translateShaders(DocumentPtr doc, StringVec& log) 
    {
        std::ignore = doc;
        std::ignore = log;
    };

    /// <summary>
    ///     Set document containing MaterialX definitions. This includes core library
    ///     definitions
    /// </summary>
    /// <param name="doc">Definition document</param>
    void setDefinitions(DocumentPtr doc)
    {
        _definitions = doc;
    }

    /// <summary>
    ///     Set document to use for MaterialX material generation or extraction.
    /// </summary>
    /// <param name="materials">MaterialX document</param>
    void setMaterials(DocumentPtr materials)
    {
        _materials = materials;
    }

    /// <summary>
    ///     Get MaterialX document containing material information
    /// </summary>
    /// <returns>MaterialX document</returns>
    DocumentPtr getMaterials() const
    {
        return _materials;
    }

    /// <summary>
    ///     Set whether to generate MaterialX assignments if found in the input glTF file.
    ///     By default assignments are not generated.
    /// </summary>
    /// <param name="val">Generate assignments flag</param>
    void setGenerateAssignments(bool val)
    {
        _generateAssignments = val;
    }

    /// <summary>
    ///     Get whether to generate MaterialX material assignments.
    /// </summary>
    /// <returns>True if generating assignments</returns>
    bool getGenerateAssignments() const
    {
        return _generateAssignments;
    }

    /// <summary>
    ///     Set whether to generate all inputs on MaterialX nodes when converting from glTF file.
    ///     By default all inputs are generated.
    /// </summary>
    /// <param name="val">Generate inputsflag</param>
    void setGenerateFullDefinitions(bool val)
    {
        _generateFullDefinitions = val;
    }

    /// <summary>
    ///     Get whether to generate all inputs for MaterialX nodes.
    /// </summary>
    /// <returns>True if generating</returns>
    bool getGenerateFullDefinitions() const
    {
        return _generateFullDefinitions;
    }

  protected:
    StringSet _extensions;
    DocumentPtr _definitions;
    DocumentPtr _materials;

    // Generation options
    bool _generateFullDefinitions;
    bool _generateAssignments;
    bool _generateNodeDefs;
};

/// @class GltfMaterialHandler
/// Wrapper for handling import / export of materials to / from GLTF files
class MX_GLTF_API GltfMaterialHandler : public MaterialHandler
{
  public:
    GltfMaterialHandler() 
        : MaterialHandler()
    {
        _extensions = { "glb", "GLB", "gltf", "GLTF" };
    }
    virtual ~GltfMaterialHandler() 
    {
        _materials = nullptr;
    }


    /// Create a new loader
    static MaterialHandlerPtr create() { return std::make_shared<GltfMaterialHandler>(); }

    /// Load materials from file path

    /// <summary>
    ///     Convert MaterialX document to glTF and save to file path
    /// </summary>
    /// <param name="filePath">File path</param>
    /// <param name="log">Error log</param>
    /// <returns>True on success</returns>
    bool load(const FilePath& filePath, StringVec& log) override;

    /// <summary>
    ///     Convert glTF to MaterialX document and save to file path    
    /// </summary>
    /// <param name="filePath">File path</param>
    /// <param name="log">Error log</param>
    /// <returns>True on success</returns>
    bool save(const FilePath& filePath, StringVec& log) override;

    /// <summary>
    ///     Translate shaders in the document to appropriate shading model(s)
    ///     for export. Derived classes can override this method
    ///     to perform actions such as shader translation and baking.
    /// </summary>
    /// <param name="doc">Document to modify</param>
    /// <param name="log">Error log</param>
    void translateShaders(DocumentPtr doc, StringVec& log) override;

  private:
    NodePtr createColoredTexture(DocumentPtr& doc, const std::string & nodeName, const std::string& fileName,
                                 const Color4& color, const std::string & colorspace);
    NodePtr createTexture(DocumentPtr& doc, const std::string & nodeName, const std::string& fileName,
                          const std::string & textureType, const std::string & colorspace, 
                          const std::string& nodeType = "gltf_image");
    void    readColorInput(DocumentPtr materials, NodePtr shaderNode, const std::string& inputName, 
                          const Color3& color, float alpha, const std::string& alphaInputName, 
                          const void* textureView, const std::string& inputImageNodeName);
    void    readFloatInput(DocumentPtr materials, NodePtr shaderNode, const std::string& inputName, 
                          float floatFactor, const void* textureView,
                          const std::string& inputImageNodeName);
    void    readVector3Input(DocumentPtr materials, NodePtr shaderNode, const std::string& inputName, 
                            const Vector3& vecFactor, const void* textureViewIn,
                            const std::string& inputImageNodeName);
    void    readNormalMapInput(DocumentPtr materials, NodePtr shaderNode, const std::string& inputName, 
                              const void* textureViewIn, const std::string& inputImageNodeName);

    void loadMaterials(void *);
};

MATERIALX_NAMESPACE_END

#endif
