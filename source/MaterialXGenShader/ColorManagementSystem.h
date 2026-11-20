//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
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

MATERIALX_NAMESPACE_BEGIN

class ShaderGenerator;

/// A shared pointer to a ColorManagementSystem
using ColorManagementSystemPtr = shared_ptr<class ColorManagementSystem>;

/// @struct ColorSpaceTransform
/// Structure that represents color space transform information
struct MX_GENSHADER_API ColorSpaceTransform
{
    ColorSpaceTransform(const string& ss, const string& ts, TypeDesc t);

    string sourceSpace;
    string targetSpace;
    TypeDesc type;

    /// Comparison operator
    bool operator==(const ColorSpaceTransform& other) const
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
    bool supportsTransform(const ColorSpaceTransform& transform) const;

    /// Returns true if the given color space name is one of the no-op color spaces
    /// ("none" and "data") reserved by the MaterialX specification, independent of
    /// any particular color management system.
    static bool isReservedNoOpColorSpace(const string& colorSpace);

    /// Returns true if no color transformation is required between the given source
    /// and target color spaces. This is the case when the two names refer to the same
    /// color space, or when either name is a no-op color space such as the reserved
    /// "none" and "data" names. The base implementation compares the names directly
    /// and consults isReservedNoOpColorSpace(). Subclasses may extend this to
    /// recognize aliases of the same color space, such as a legacy name and its color
    /// interop equivalent, or additional no-op color spaces known to the color
    /// management system, and should call the base implementation as well.
    virtual bool isNoOpTransform(const string& sourceColorSpace, const string& targetColorSpace) const
    {
        return sourceColorSpace == targetColorSpace ||
               isReservedNoOpColorSpace(sourceColorSpace) ||
               isReservedNoOpColorSpace(targetColorSpace);
    }

    /// The colorSpace strings should not be shown directly in a user interface. This function
    /// converts a colorSpace into a user-facing name. For example, "lin_rec709_scene" becomes
    /// "Linear Rec.709 (sRGB)". If the colorSpace is not recognized, it is returned unchanged.
    virtual string getUserFacingName(const string& colorSpace) const { return colorSpace; }

    /// Create a node to use to perform the given color space transformation.
    ShaderNodePtr createNode(const ShaderGraph* parent, const ColorSpaceTransform& transform, const string& name,
                             GenContext& context) const;

    /// Returns true if the CMS can create a shader node implementation for a locally managed CMS transform
    virtual bool hasImplementation(const string& /*implName*/) const { return false; }

    /// Create an CMS node implementation for a locally managed transform
    virtual ShaderNodeImplPtr createImplementation(const string& /*implName*/) const { return {}; }

  protected:
    /// Protected constructor
    ColorManagementSystem();

    /// Returns a nodedef for a given transform
    virtual NodeDefPtr getNodeDef(const ColorSpaceTransform& transform) const = 0;

  protected:
    DocumentPtr _document;
};

MATERIALX_NAMESPACE_END

#endif
