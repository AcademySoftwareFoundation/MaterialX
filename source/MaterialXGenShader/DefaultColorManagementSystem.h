#ifndef MATERIALX_DEFAULT_COLOR_MANAGEMENT_SYSTEM_H
#define MATERIALX_DEFAULT_COLOR_MANAGEMENT_SYSTEM_H

#include <MaterialXGenShader/ColorManagementSystem.h>

namespace MaterialX
{

/// A shared pointer to a DefaultColorManagementSystem
using DefaultColorManagementSystemPtr = shared_ptr<class DefaultColorManagementSystem>;

/// @class @DefaultColorManagementSystem
/// Class for a default color management system. The default color management system users
/// the typical workflow for registering nodes with the exception that the target for them
/// nodedefs is set to DefaultColorManagementSystem::CMS_NAME.
///
class DefaultColorManagementSystem : public ColorManagementSystem
{
  public:
    /// Create a new DefaultColorManagementSystem
    static DefaultColorManagementSystemPtr create(const string& language);

    /// Return the DefaultColorManagementSystem name
    const string& getName() const override
    {
        return DefaultColorManagementSystem::CMS_NAME;
    }

    /// Returns an implementation name for a given transform
    string getImplementationName(const ColorSpaceTransform& transform) override;

    static const string CMS_NAME;

  private:
    /// Protected constructor
    DefaultColorManagementSystem(const string& language);

    string _language;
};

} // namespace MaterialX

#endif
