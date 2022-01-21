//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_ENVIRON_H
#define MATERIALX_ENVIRON_H

/// @file
/// Cross-platform environment variable functionality

#include <MaterialXCore/Library.h>

#include <MaterialXFormat/Export.h>

MATERIALX_NAMESPACE_BEGIN

/// Return the value of an environment variable by name
MX_FORMAT_API string getEnviron(const string& name);

/// Set an environment variable to a specified value
MX_FORMAT_API bool setEnviron(const string& name, const string& value);

/// Remove an environment variable by name
MX_FORMAT_API bool removeEnviron(const string& name);

MATERIALX_NAMESPACE_END

#endif // MATERIALX_ENVIRON_H
