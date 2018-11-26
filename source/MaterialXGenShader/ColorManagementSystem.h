#ifndef MATERIALX_COLOR_MANAGEMENT_SYSTEM_H
#define MATERIALX_COLOR_MANAGEMENT_SYSTEM_H

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Library.h>

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

/// @struct @ColorSpaceTransformHash
/// ColorSpaceTransform hash function
struct ColorSpaceTransformHash {
    size_t operator()(const ColorSpaceTransform &transform ) const
    {
        return std::hash<string>()(transform.sourceSpace + transform.targetSpace + transform.type->getName());
    }
};

/// @class @ColorManagementSystem
/// Abstract base class for a ColorManagementSystem.
///
class ColorManagementSystem
{
  public:
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
    bool supportsTransform(const ColorSpaceTransform& transform);

    /// Create a node to use to perform the given color space transformation.
    ShaderNodePtr createNode(const ColorSpaceTransform& transform, const string& name, ShaderGenerator& shadergen, const GenOptions& options);

  protected:
    template<class T>
    using CreatorFunction = shared_ptr<T>(*)();

    /// Returns an implementation name for a given transform
    virtual string getImplementationName(const ColorSpaceTransform& transform) = 0;

    /// Register a node implementation for a given color space transformation.
    void registerImplementation(const ColorSpaceTransform& transform, CreatorFunction<ShaderNodeImpl> creator);

    /// Protected constructor
    ColorManagementSystem(const string& configFile);

    Factory<ShaderNodeImpl> _implFactory;
    std::unordered_map<ColorSpaceTransform, ShaderNodeImplPtr, ColorSpaceTransformHash> _cachedImpls;
    vector<string> _registeredImplNames;
    string _configFile;
    DocumentPtr _document;
};

} // namespace MaterialX

#endif
