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
