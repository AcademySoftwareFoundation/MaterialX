#ifndef MATERIALX_SHADERGEN_UTIL_H
#define MATERIALX_SHADERGEN_UTIL_H

/// Utility methods

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Element.h>
#include <MaterialXCore/Interface.h>
#include <MaterialXCore/Document.h>

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

/// Maps a value to a four channel color if it is of the appropriate type.
/// Supported types include float, Vector2, Vector3, Vector4,
/// Color2, and Color4. Note that for Color2 the second channel
/// maps to alpha. If not mapping is possible the color value is
/// set to opaque black.
void mapValueToColor(const ValuePtr value, Color4& color);

/// Return whether a nodedef requires an implementation
bool requiresImplementation(const NodeDefPtr nodeDef);

/// Determine if a given element requires shading / lighting for rendering
bool elementRequiresShading(const TypedElementPtr element);

/// Find any elements which may be renderable from within a document.
/// This includes all outputs on node graphs and shader references which are not
/// part of any included library. Light shaders are not considered to be renderable.
void findRenderableElements(const DocumentPtr& doc, std::vector<TypedElementPtr>& elements);

} // namespace MaterialX

#endif
