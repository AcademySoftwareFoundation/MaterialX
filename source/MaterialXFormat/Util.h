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
void loadLibrary(const FilePath& file,
                 DocumentPtr doc,
                 const FileSearchPath& searchPath = FileSearchPath(), 
                 XmlReadOptions* readOptions = nullptr);

/// Load all MaterialX files within the given library folders into a document,
/// using the given search path to locate the folders on the file system.
StringSet loadLibraries(const FilePathVec& libraryFolders,
                        const FileSearchPath& searchPath,
                        DocumentPtr doc,
                        const StringSet& excludeFiles = StringSet(),
                        XmlReadOptions* readOptions = nullptr);

/// Set file name values to resolved values. 
/// The resolve applies:
///   - the Element's default string resolver
///   - an optional relative to absoulte path conversion
///   - an optional custom file name string resolver
/// Note that all "fileprefix" attributes will be removed from the Document as they have
/// been prepended to the resolved file name values.
/// \param doc Document to modify.
/// \param searchPath Optional search path for relative to absolute path conversion. Default value is an empty search path.
/// \param customResolver Optional custom name resolver to apply. Default value is a null resolver.
void resolveFileNames(DocumentPtr doc, const FileSearchPath& searchPath = FileSearchPath(), StringResolverPtr customResolver = nullptr);

} // namespace MaterialX

#endif
