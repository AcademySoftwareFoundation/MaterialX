#ifndef MATERIALX_SHADERGEN_UTIL_H
#define MATERIALX_SHADERGEN_UTIL_H

/// Utility methods

#include <MaterialXCore/Library.h>

namespace MaterialX
{

/// Reads the contents of a file into the given string
bool readFile(const string& filename, string& content);

/// Returns the extension of the given filename
string getFileExtension(const string& filename);

} // namespace MaterialX

#endif
