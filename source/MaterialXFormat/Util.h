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
void loadDocuments(const FilePath& rootPath, const FileSearchPath& searchPath, const StringSet& skipFiles,
                   const StringSet& includeFiles, vector<DocumentPtr>& documents, StringVec& documentsPaths,
                   const XmlReadOptions& readOptions, StringVec& errors);

/// Load a given MaterialX library into a document
void loadLibrary(const FilePath& file, DocumentPtr doc, const FileSearchPath* searchPath = nullptr, 
                 XmlReadOptions* readOptions = nullptr);

/// Load all MaterialX files with given library names in given search paths.
/// Note that all library files will have a URI set on them.
StringSet loadLibraries(const FilePathVec& libraryNames,
                        const FileSearchPath& searchPath,
                        DocumentPtr doc,
                        const StringSet* excludeFiles = nullptr,
                        XmlReadOptions* readOptions = nullptr);

} // namespace MaterialX

#endif
