//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_COLOR_MANAGEMENT_SYSTEM_H
#define MATERIALX_COLOR_MANAGEMENT_SYSTEM_H

/// @file
/// Color management system classes

#include <MaterialXGenShader/Export.h>

#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/TypeDesc.h>

#include <MaterialXCore/Document.h>

namespace MaterialX
{

class ShaderGenerator;

/// A shared pointer to a ColorManagementSystem
using ColorManagementSystemPtr = shared_ptr<class ColorManagementSystem>;

/// Set of mappings between a uniform name and it's associated value
using ColorSpaceUniformPtr = shared_ptr<class ColorSpaceUniform>;
class MX_GENSHADER_API ColorSpaceUniform
{
  public:
    ColorSpaceUniform(const string name)
        : _name(name)
    {
    }

    string _name;
};

/// Set of mappings between a uniform name and it's associated value
using ColorSpaceConstantPtr = shared_ptr<class ColorSpaceConstant>;
class MX_GENSHADER_API ColorSpaceConstant : public ColorSpaceUniform
{
  public:
    static ColorSpaceConstantPtr create(const std::string& name, const ValuePtr value);

    ColorSpaceConstant(const string name, const ValuePtr value);

    ValuePtr _value;
};


using ColorSpaceTexturePtr = shared_ptr<class ColorSpaceTexture>;
class MX_GENSHADER_API ColorSpaceTexture : public ColorSpaceUniform
{
  public:
    static ColorSpaceTexturePtr create(const std::string& name, const FloatVec& data);

    ColorSpaceTexture(const string name, const FloatVec& data);

    // TBD : Could replace this with Image
    FloatVec _data;
    unsigned int _channelCount = 3; // 1 or 3
    unsigned int _width = 0;
    unsigned int _height = 0;
    unsigned int _depth = 0;
};

using ColorManagementResourceMap = std::unordered_map<std::string, ColorSpaceUniformPtr>;
using ColorManagementResourceMapPtr = shared_ptr<ColorManagementResourceMap>;

/// @struct ColorSpaceTransform
/// Structure that represents color space transform information
struct MX_GENSHADER_API ColorSpaceTransform
{
    ColorSpaceTransform(const string& ss, const string& ts, const TypeDesc* t);

    string sourceSpace;
    string targetSpace;
    const TypeDesc* type;

    /// Comparison operator
    bool operator==(const ColorSpaceTransform &other) const
    {
        return sourceSpace == other.sourceSpace &&
               targetSpace == other.targetSpace &&
               type == other.type;
    }
};

/// @class ColorManagementSystem
/// Abstract base class for color management systems
class MX_GENSHADER_API ColorManagementSystem
{
  public:
    enum class ResourceType : int
    {
        UNIFORM = 0,
        TEXTURE1D = 1,
        TEXTURE2D = 2,
        TEXTURE3D = 3
    };

    virtual ~ColorManagementSystem() { }

    /// Return the ColorManagementSystem name
    virtual const string& getName() const = 0;

    /// Load a library of implementations from the provided document,
    /// replacing any previously loaded content.
    virtual void loadLibrary(DocumentPtr document);

    /// Returns whether this color management system supports a provided transform
    virtual bool supportsTransform(const ColorSpaceTransform& transform) const;

    /// Create a node to use to perform the given color space transformation.
    virtual ShaderNodePtr createNode(const ShaderGraph* parent, const ColorSpaceTransform& transform, const string& name,
                                     GenContext& context) const;

    /// Connect node to graph input
    void connectNodeToShaderInput(ShaderGraph* graph, ShaderNode* node, ShaderInput* shasderInput, GenContext& context);

    /// Connect node to graph output
    void connectNodeToShaderOutput(ShaderGraph* graph, ShaderNode* node, ShaderOutput* shaderOutput, GenContext& context);

    /// Get resource information to bind with
    virtual const ColorManagementResourceMapPtr getResource(ResourceType /*resourceType*/) const
    {
        return nullptr;
    }

    /// Clear the resource information 
    virtual void clearResources() 
    {
    }

    /// Name of block for color management uniforms
    static string COLOR_MANAGEMENT_UNIFORMS;

  protected:
    /// Protected constructor
    ColorManagementSystem();
      
    /// Returns an implementation for a given transform
    virtual ImplementationPtr getImplementation(const ColorSpaceTransform& transform) const = 0;

    /// For a given transform node and a target port type to connect to, determine the appropriate
    /// input and output ports. If the transform node's port type does not match the target type
    /// additional "conversion" nodes will created and the appropriate input and output ports on these 
    /// nodes will be returned.
    ///
    /// The possible configurations include two cases where additional conversion nodes are not required:
    /// 
    ///  color3 input  -> color3 transform node -> color3 output
    ///  color4 input  -> color4 transform node -> color3 output
    /// 
    /// and the following two cases where they are:
    ///
    ///  color3 input -> color3-to-color4 conversion -> color4 transform node -> color4_to_color3 conversion -> color3 output
    ///  color4 input -> color4-to-color3 conversion -> color3 transform node -> color3-to-color4 conversion -> color4 output
    ///
    virtual void getPortConnections(ShaderGraph* graph, ShaderNode* colorTransformNode,
                                    const TypeDesc* targetType, GenContext& context,
                                    ShaderInput*& inputToConnect, ShaderOutput*& outputToConnect);

  protected:
    DocumentPtr _document;
};

} // namespace MaterialX

#endif
