//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_DEFAULT_COLOR_MANAGEMENT_SYSTEM_H
#define MATERIALX_DEFAULT_COLOR_MANAGEMENT_SYSTEM_H

/// @file
/// Default color management system implementation

#include <MaterialXGenShader/ColorManagementSystem.h>

namespace MaterialX
{

/// A shared pointer to a DefaultColorManagementSystem
using DefaultColorManagementSystemPtr = shared_ptr<class DefaultColorManagementSystem>;

/// @class DefaultColorManagementSystem
/// Class for a default color management system. The default color management system uses
/// the typical workflow for registering nodes with the exception that the target for the
/// nodedefs is set to DefaultColorManagementSystem::CMS_NAME.
///
class MX_GENSHADER_API DefaultColorManagementSystem : public ColorManagementSystem
{
  public:
    virtual ~DefaultColorManagementSystem() { }

    /// Create a new DefaultColorManagementSystem
    static DefaultColorManagementSystemPtr create(const string& target);

    /// Return the DefaultColorManagementSystem name
    const string& getName() const override;

  protected:
    /// Returns an implementation for a given transform
    ImplementationPtr getImplementation(const ColorSpaceTransform& transform) const override;

    /// Protected constructor
    DefaultColorManagementSystem(const string& target);

  private:
    string _target;
};

} // namespace MaterialX

#endif
