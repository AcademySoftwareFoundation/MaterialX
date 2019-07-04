#ifndef UTILS_H
#define UTILS_H

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Interface.h>
#include <MaterialXFormat/File.h>

namespace mx = MaterialX;

namespace MaterialXMaya
{

/// Load a given MaterialX file into a document
void loadLibrary(const mx::FilePath& file, mx::DocumentPtr doc);

/// Load all MaterialX files with a given set of library name into a document..
/// Note that all library files will have a URI set on them.
void loadLibraries(const mx::StringVec& libraryNames,
                   const mx::FileSearchPath& searchPath,
                   mx::DocumentPtr doc,
                   const mx::StringSet* excludeFiles = nullptr);

/// Find a given file path under a set of search paths. The search is performed
/// on all subdirectories for each search path
mx::FilePath findInSubdirectories(const mx::FileSearchPath& searchPaths,
                                  const mx::FilePath& filePath);

mx::DocumentPtr loadDocument(const std::string& materialXDocumentPath,
                             const MaterialX::FileSearchPath& librarySearchPath);

} // namespace MaterialXMaya

#endif
