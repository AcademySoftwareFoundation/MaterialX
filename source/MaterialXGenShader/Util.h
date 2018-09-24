#ifndef MATERIALX_SHADERGEN_UTIL_H
#define MATERIALX_SHADERGEN_UTIL_H

/// Utility methods

#include <MaterialXCore/Library.h>

namespace MaterialX
{

/// Make the directory with the given path if it doesn't already exist
void makeDirectory(const std::string& directoryPath);

/// Removes the extension from the provided filename
std::string removeExtension(const std::string& filename);

/// Directory scanner utility. Finds all subdirectories in the given directory
void getSubDirectories(const std::string& baseDirectory, StringVec& relativePaths);

/// Directory file scanner utility. Finds all files with a given extension
/// in the given directory.
void getFilesInDirectory(const std::string& directory, StringVec& files, const std::string& extension);

/// Reads the contents of a file into the given string
bool readFile(const string& filename, string& content);

/// Returns the extension of the given filename
string getFileExtension(const string& filename);

} // namespace MaterialX

#endif
