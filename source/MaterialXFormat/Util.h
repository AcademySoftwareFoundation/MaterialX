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

#include <MaterialXFormat/Export.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/XmlIo.h>

namespace MaterialX
{

extern MX_FORMAT_API const string PATH_LIST_SEPARATOR;
extern MX_FORMAT_API const string MATERIALX_SEARCH_PATH_ENV_VAR;
extern MX_FORMAT_API const string MATERIALX_ASSET_DEFINITION_PATH_ENV_VAR;
extern MX_FORMAT_API const string MATERIALX_ASSET_TEXTURE_PATH_ENV_VAR;

/// Read the given file and return a string containing its contents; if the read is not
/// successful, then the empty string is returned.
MX_FORMAT_API string readFile(const FilePath& file);

/// Get all subdirectories for a given set of directories and search paths
MX_FORMAT_API void getSubdirectories(const FilePathVec& rootDirectories, const FileSearchPath& searchPath, FilePathVec& subDirectories);

/// Scans for all documents under a root path and returns documents which can be loaded
MX_FORMAT_API void loadDocuments(const FilePath& rootPath,
                   const FileSearchPath& searchPath,
                   const StringSet& skipFiles,
                   const StringSet& includeFiles,
                   vector<DocumentPtr>& documents,
                   StringVec& documentsPaths,
                   const XmlReadOptions* readOptions = nullptr,
                   StringVec* errors = nullptr);

/// Load a given MaterialX library into a document
MX_FORMAT_API void loadLibrary(const FilePath& file,
                 DocumentPtr doc,
                 const FileSearchPath& searchPath = FileSearchPath(), 
                 const XmlReadOptions* readOptions = nullptr);

/// Load all MaterialX files within the given library folders into a document,
/// using the given search path to locate the folders on the file system.
MX_FORMAT_API StringSet loadLibraries(const FilePathVec& libraryFolders,
                        const FileSearchPath& searchPath,
                        DocumentPtr doc,
                        const StringSet& excludeFiles = StringSet(),
                        const XmlReadOptions* readOptions = nullptr);

/// Load the core library files within the given library folders into a document,
/// using the given search path to locate the folders on the file system.
MX_FORMAT_API StringSet loadCoreLibraries(const FilePathVec& libraryFolders,
                        const FileSearchPath& searchPath,
                        DocumentPtr doc,
                        const StringSet& excludeFiles = StringSet(),
                        XmlReadOptions* readOptions = nullptr);

/// Flatten all filenames in the given document, applying string resolvers at the
/// scope of each element and removing all fileprefix attributes.
/// @param doc The document to modify.
/// @param searchPath An optional search path for relative to absolute path conversion.
/// @param customResolver An optional custom resolver to apply.
MX_FORMAT_API void flattenFilenames(DocumentPtr doc, const FileSearchPath& searchPath = FileSearchPath(), StringResolverPtr customResolver = nullptr);

MX_FORMAT_API FileSearchPath getEnvironmentPath(const string& sep = PATH_LIST_SEPARATOR);
MX_FORMAT_API FileSearchPath getAssetDefinitionPath(const string& sep = PATH_LIST_SEPARATOR);
MX_FORMAT_API FileSearchPath getAssetTexturePath(const string& sep = PATH_LIST_SEPARATOR);
MX_FORMAT_API FileSearchPath getCoreDefinitionPath();
MX_FORMAT_API void setEnvironmentPath(const FileSearchPath& path);
MX_FORMAT_API void setAssetDefinitionPath(const FileSearchPath& path);
MX_FORMAT_API void setAssetTexturePath(const FileSearchPath& path);
MX_FORMAT_API void setCoreDefinitionPath(const FileSearchPath& path);

} // namespace MaterialX

#endif
