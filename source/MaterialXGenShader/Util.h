#ifndef MATERIALX_SHADERGEN_UTIL_H
#define MATERIALX_SHADERGEN_UTIL_H

/// Utility methods

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Element.h>
#include <MaterialXCore/Interface.h>

namespace MaterialX
{

class ShaderGenerator;

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

/// Returns true if the given element is a surface shader with the potential
/// of beeing transparent. This can be used by HW shader generators to determine
/// if a shader will require transparency handling.
///
/// Note: This function will check some common cases for how a surface
/// shader can be transparent. It is not covering all possible cases for
/// how transparency can be done and target applications might need to do
/// additional checks to track transparency correctly. For example, custom
/// surface shader nodes implemented in source code will not be tracked by this
/// function and transprency for such nodes must be tracked separately by the 
/// target application.
///
bool isTransparentSurface(ElementPtr element, const ShaderGenerator& shadergen);

///
/// Given a nodedef and corresponding implementation, return the
/// implementation value if any for a value.
///
/// An implementation value will be returned if:
/// - There is a implementation Parametner with the same name as the input Value 
/// - There is a nodedef Value with the same name as the input Value 
/// - There is a enumeration and type specified on the implementation Parameter 
/// - There is a enumeration and type specified on the nodedef Value
///
/// @param elem Value element input
/// @param impl Implementation to use
/// @param nodeDef Node definition to use
/// @param implType Implementation type (if any) specified.
/// @return Implementation value. Null if could not be evaluated
///
ValuePtr getImplementationValue(const ValueElementPtr& elem, const InterfaceElementPtr impl, const NodeDef& nodeDef,
                                string& implType);


} // namespace MaterialX

#endif
