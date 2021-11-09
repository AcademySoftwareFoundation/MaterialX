//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTVERSIONRESOLVER_H
#define MATERIALX_RTVERSIONRESOLVER_H

/// @file
/// Publishing version formatting methods

#include <MaterialXCore/Export.h>

namespace MaterialX
{

/// Validate if the give string is a proper version format.
MX_CORE_API bool isValidVersionFormat(const string& versionFormat);

/// Validate if the give string is a proper version format like prefix#postfix
MX_CORE_API bool isValidIntegerVersionFormat(const string& versionFormat);

/// Validate if the give string is a proper version format like prefix.###postfix 
MX_CORE_API bool isValidFloatVersionFormat(const string& versionFormat);

/// Given the version number and version format return the formatted version string
MX_CORE_API string getFormattedVersionString(const string& versionNumber, const string& versionFormat);

/// Returns the decimal precision of the given version format. i.e. for "#" the result is 0, but for ".##" the result is 2.
MX_CORE_API int getVersionFormatDecimalPrecision(const string& versionFormat);

/// Returns the increment step based on the decimal precision. If the decimal precision is zero,
/// then we are dealing with integer version format and increment step equals 1. In case we have a
/// decimal precision of 2, then the increment step is 0.01
MX_CORE_API double getVersionIncrementStep(const int decimalPrecision);

} // namespace MaterialX

#endif