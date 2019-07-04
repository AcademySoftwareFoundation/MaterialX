#include "MaterialXUtil.h"

#include <MaterialXCore/Traversal.h>
#include <MaterialXFormat/XmlIo.h>

#include <iostream>

namespace mx = MaterialX;

namespace MaterialXMaya
{

void loadLibrary(const mx::FilePath& filePath, mx::DocumentPtr doc)
{
	mx::DocumentPtr libDoc = mx::createDocument();
	mx::readFromXmlFile(libDoc, filePath, mx::EMPTY_STRING);
    mx::CopyOptions copyOptions;
    copyOptions.skipDuplicateElements = true;
    doc->importLibrary(libDoc, &copyOptions);
}

void loadLibraries(const mx::StringVec& libraryNames,
	               const mx::FileSearchPath& searchPath,
	               mx::DocumentPtr doc,
	               const mx::StringSet* excludeFiles)
{
	for (const std::string& libraryName : libraryNames)
	{
		mx::FilePath libraryPath = searchPath.find(libraryName);
		for (const mx::FilePath& path : libraryPath.getSubDirectories())
		{
			for (const mx::FilePath& filename : path.getFilesInDirectory(mx::MTLX_EXTENSION))
			{
				if (!excludeFiles || !excludeFiles->count(filename))
				{
					loadLibrary(path / filename, doc);
				}
			}
		}
	}
}

mx::FilePath findInSubdirectories(const mx::FileSearchPath& searchPaths,
                                  const mx::FilePath& filePath)
{
    mx::FilePath foundPath;
    for (size_t i = 0; i < searchPaths.size(); i++)
    {
        for (const mx::FilePath& directory : searchPaths[i].getSubDirectories())
        {
            mx::FileSearchPath searchPath(directory);
            foundPath = searchPath.find(filePath);
            if (foundPath.exists())
            {
                return foundPath;
            }
        }
    }
    return foundPath;
}

mx::DocumentPtr loadDocument(const std::string& materialXDocumentPath,
                             const MaterialX::FileSearchPath& librarySearchPath)
{
    // Create document
    mx::DocumentPtr document = mx::createDocument();
    if (!document)
    {
        throw mx::Exception("Failed to create a MaterialX document");
    }

    // Load libraries
    static const mx::StringVec libraries = { "stdlib", "pbrlib", "bxdf", "stdlib/genglsl", "pbrlib/genglsl" };
    MaterialXMaya::loadLibraries(libraries, librarySearchPath, document);

    // Read document contents from disk
    mx::readFromXmlFile(document, materialXDocumentPath, mx::EMPTY_STRING);

    return document;
}

} // namespace MaterialXMaya

