//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_ENVIRON_H
#define MATERIALX_ENVIRON_H

/// @file
/// Cross-platform environment variable functionality

#include <MaterialXCore/Library.h>

namespace MaterialX
{

/// Return the value of an environment variable by name
string getEnviron(const string& name);

/// Set an environment variable to a specified value
bool setEnviron(const string& name, const string& value);

/// Remove an environment variable by name
bool removeEnviron(const string& name);

} // namespace MaterialX

#endif // MATERIALX_ENVIRON_H
