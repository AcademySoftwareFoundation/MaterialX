#include "MaterialXUtil.h"

#include <MaterialXCore/Traversal.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXGenShader/Util.h>

#include <iostream>

namespace mx = MaterialX;

namespace MaterialXMaya
{

namespace MaterialXUtil
{

mx::FilePath findInSubdirectories(const mx::FileSearchPath& searchPaths, const mx::FilePath& filePath)
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

mx::DocumentPtr loadDocument(const std::string& materialXDocumentPath, mx::ConstDocumentPtr libraryDocument)
{
    // Create document
    mx::DocumentPtr document = mx::createDocument();
    if (!document)
    {
        throw mx::Exception("Failed to create a MaterialX document");
    }

    // Import libraries.
    document->importLibrary(libraryDocument);

    // Read document contents from disk
    mx::XmlReadOptions readOptions;
    readOptions.applyFutureUpdates = true;
    mx::readFromXmlFile(document, materialXDocumentPath, mx::EMPTY_STRING, &readOptions);

    return document;
}

mx::TypedElementPtr getRenderableElement(mx::DocumentPtr document,
                                         const std::vector<mx::TypedElementPtr>& renderableElements,
                                         const std::string& desiredElementPath)
{
    if (!desiredElementPath.empty())
    {
        mx::ElementPtr element = document->getDescendant(desiredElementPath);
        if (!element)
        {
            throw mx::Exception("The specified element " + desiredElementPath + " does not exist in the document");
        }

        auto it = std::find_if(renderableElements.begin(), renderableElements.end(),
                               [element](mx::TypedElementPtr renderableElement) -> bool {
                                   return (element->getNamePath() == renderableElement->getNamePath());
                               });

        if (it == renderableElements.end())
        {
            throw mx::Exception("The specified element " + desiredElementPath + " is not renderable");
        }

        return element->asA<mx::TypedElement>();
    }
    return nullptr;
}

} // namespace MaterialXUtil
} // namespace MaterialXMaya
