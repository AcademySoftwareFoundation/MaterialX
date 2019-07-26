#ifndef MATERIALX_UTILS_H
#define MATERIALX_UTILS_H

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Interface.h>
#include <MaterialXFormat/File.h>

namespace mx = MaterialX;

namespace MaterialXMaya
{
namespace MaterialXUtil
{

/// Find a given file path under a set of search paths. The search is performed
/// on all subdirectories for each search path
mx::FilePath findInSubdirectories(const mx::FileSearchPath& searchPaths,
                                  const mx::FilePath& filePath);

/// Load in a document and associated libraries from library search path
mx::DocumentPtr loadDocument(const std::string& materialXDocumentPath,
                             const mx::FileSearchPath& librarySearchPath);

/// Given an element path return a pointer to it within a document if it is considered to be renderable.
/// @param document Document to examine
/// @param renderableElements List of elements in the document that are considered to be renderable.
/// @param elementPath Path to element to find
mx::TypedElementPtr getRenderableElement(mx::DocumentPtr document,
                                        const std::vector<mx::TypedElementPtr>& renderableElements,
                                        const std::string& desiredElementPath);

} // namespace MaterialXUtil
} // namespace MaterialXMaya

#endif
