//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_FORMAT_UTIL_H
#define MATERIALX_FORMAT_UTIL_H

/// @file
/// Format utility methods

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Element.h>
#include <MaterialXCore/Interface.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/XmlIo.h>

namespace MaterialX
{

/// Read the given file and return a string containing its contents; if the read is not
/// successful, then the empty string is returned.
string readFile(const FilePath& file);

/// Get all subdirectories for a given set of directories and search paths
void getSubdirectories(const FilePathVec& rootDirectories, const FileSearchPath& searchPath, FilePathVec& subDirectories);

/// Scans for all documents under a root path and returns documents which can be loaded
void loadDocuments(const FilePath& rootPath,
                   const FileSearchPath& searchPath,
                   const StringSet& skipFiles,
                   const StringSet& includeFiles,
                   vector<DocumentPtr>& documents,
                   StringVec& documentsPaths,
                   const XmlReadOptions* readOptions = nullptr,
                   StringVec* errors = nullptr);

/// Load a given MaterialX library into a document
void loadLibrary(const FilePath& file,
                 DocumentPtr doc,
                 const FileSearchPath& searchPath = FileSearchPath(), 
                 const XmlReadOptions* readOptions = nullptr);

/// Load all MaterialX files within the given library folders into a document,
/// using the given search path to locate the folders on the file system.
StringSet loadLibraries(const FilePathVec& libraryFolders,
                        const FileSearchPath& searchPath,
                        DocumentPtr doc,
                        const StringSet& excludeFiles = StringSet(),
                        const XmlReadOptions* readOptions = nullptr);

/// Flatten all filenames in the given document, applying string resolvers at the
/// scope of each element and removing all fileprefix attributes.
/// @param doc The document to modify.
/// @param searchPath An optional search path for relative to absolute path conversion.
/// @param customResolver An optional custom resolver to apply.
void flattenFilenames(DocumentPtr doc, const FileSearchPath& searchPath = FileSearchPath(), StringResolverPtr customResolver = nullptr);

} // namespace MaterialX

#endif
