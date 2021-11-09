//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTVERSIONRESOLVER_H
#define MATERIALX_RTVERSIONRESOLVER_H

/// @file
/// Publishing version formatting methods

#include <string>

namespace MaterialX
{

/// Validate if the give string is a proper version format.
bool isValidVersionFormat(const std::string& versionFormat);

/// Validate if the give string is a proper version format like prefix#postfix
bool isValidIntegerVersionFormat(const std::string& versionFormat);

/// Validate if the give string is a proper version format like prefix.###postfix 
bool isValidFloatVersionFormat(const std::string& versionFormat);

/// Given the version number and version format return the formatted version string
std::string getFormattedVersionString(const std::string& versionNumber, const std::string& versionFormat);

/// Returns the decimal precision of the given version format. i.e. for "#" the result is 0, but for ".##" the result is 2.
int getVersionFormatDecimalPrecision(const std::string& versionFormat);

/// Returns the increment step based on the decimal precision. If the decimal precision is zero,
/// then we are dealing with integer version format and increment step equals 1. In case we have a
/// decimal precision of 2, then the increment step is 0.01
double getVersionIncrementStep(const int decimalPrecision);

} // namespace MaterialX

#endif