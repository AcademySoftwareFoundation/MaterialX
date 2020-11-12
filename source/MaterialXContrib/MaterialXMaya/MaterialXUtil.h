#ifndef MATERIALX_MAYA_MATERIALXUTIL_H
#define MATERIALX_MAYA_MATERIALXUTIL_H

/// @file
/// MaterialX utilities.

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
mx::FilePath findInSubdirectories(const mx::FileSearchPath& searchPaths, const mx::FilePath& filePath);

/// Load in a document and import associated libraries.
mx::DocumentPtr loadDocument(const std::string& materialXDocumentPath, mx::ConstDocumentPtr libraryDocument);

/// Given an element path return a pointer to the element within a document if
/// it is considered to be renderable.
/// @param document Document to examine.
/// @param renderableElements List of elements in the document that are considered to be renderable.
/// @param desiredElementPath Path to element to find.
mx::TypedElementPtr getRenderableElement(mx::DocumentPtr document,
                                         const std::vector<mx::TypedElementPtr>& renderableElements,
                                         const std::string& desiredElementPath);

} // namespace MaterialXUtil
} // namespace MaterialXMaya

#endif
