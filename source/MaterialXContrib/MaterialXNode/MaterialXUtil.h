#ifndef UTILS_H
#define UTILS_H

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Interface.h>
#include <MaterialXFormat/File.h>

namespace mx = MaterialX;

namespace MaterialXMaya
{

/// Find a given file path under a set of search paths. The search is performed
/// on all subdirectories for each search path
mx::FilePath findInSubdirectories(const mx::FileSearchPath& searchPaths,
                                  const mx::FilePath& filePath);

mx::DocumentPtr loadDocument(const std::string& materialXDocumentPath,
                             const MaterialX::FileSearchPath& librarySearchPath);

} // namespace MaterialXMaya

#endif
