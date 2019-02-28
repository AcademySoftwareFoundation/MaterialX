#ifndef MATERIALX_COLOR_MANAGEMENT_SYSTEM_H
#define MATERIALX_COLOR_MANAGEMENT_SYSTEM_H

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Library.h>

#include <MaterialXGenShader/Library.h>
#include <MaterialXGenShader/Factory.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/TypeDesc.h>

namespace MaterialX
{

class ShaderGenerator;

/// A shared pointer to a ColorManagementSystem
using ColorManagementSystemPtr = shared_ptr<class ColorManagementSystem>;

/// @struct @ColorSpaceTransform
/// Structure that represents color space transform information
struct ColorSpaceTransform
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

/// @class @ColorManagementSystem
/// Abstract base class for a ColorManagementSystem.
///
class ColorManagementSystem
{
  public:
    virtual ~ColorManagementSystem() {}

    /// Return the ColorManagementSystem name
    virtual const string& getName() const = 0;

    /// Return the ColorManagementSystem configFile
    const string& getConfigFile() const
    {
        return _configFile;
    }

    /// Sets the config file.
    void setConfigFile(const string& configFile);

    /// Load a library of implementations from the provided document,
    /// replacing any previously loaded content.
    virtual void loadLibrary(DocumentPtr document);

    /// Returns whether this color management system supports a provided transform
    bool supportsTransform(const ColorSpaceTransform& transform) const;

    /// Create a node to use to perform the given color space transformation.
    ShaderNodePtr createNode(const ShaderGraph* parent, const ColorSpaceTransform& transform, const string& name,
                             const ShaderGenerator& shadergen, GenContext& context) const;

  protected:
    /// Protected constructor
    ColorManagementSystem(const string& configFile);
      
    /// Returns an implementation name for a given transform
    virtual string getImplementationName(const ColorSpaceTransform& transform) const = 0;

    /// Register a node implementation for a given color space transformation.
    void registerImplementation(const ColorSpaceTransform& transform, CreatorFunction<ShaderNodeImpl> creator);

    Factory<ShaderNodeImpl> _implFactory;
    vector<string> _registeredImplNames;
    string _configFile;
    DocumentPtr _document;
};

} // namespace MaterialX

#endif
