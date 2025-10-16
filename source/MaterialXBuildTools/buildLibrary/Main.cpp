//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXCore/Exception.h>
#include <MaterialXCore/Util.h>

#include <iostream>

namespace mx = MaterialX;

const std::string cmdLineOptions =
    " Options: \n"
    "    --sourceLibraryRoot [FILEPATH]     Directory containing the source data library files.\n"
    "    --destLibraryRoot [FILEPATH]       Directory to write the modified data library files.\n"
    "    --help                             Display the complete list of command-line options\n";

template <class T> void parseToken(std::string token, std::string type, T& res)
{
    if (token.empty())
    {
        return;
    }

    mx::ValuePtr value = mx::Value::createValueFromStrings(token, type);
    if (!value)
    {
        std::cout << "Unable to parse token " << token << " as type " << type << std::endl;
        return;
    }

    res = value->asA<T>();
}

mx::FilePathVec getMaterialXFiles(const mx::FilePath& libraryRoot)
{
    mx::FilePathVec result;

    auto subDirs = libraryRoot.getSubDirectories();
    for (const auto& subDir : subDirs)
    {
        for (const auto& file : subDir.getFilesInDirectory("mtlx"))
        {
            auto absFile = subDir / file;
            std::string absFileStr = absFile.asString();
            std::string relFileStr = absFileStr.substr(libraryRoot.asString().size()+1);
            result.emplace_back(mx::FilePath(relFileStr));
        }
    }
    return result;
}

// replace named value "Constant:" strings with concrete values
void replaceConstantValues(mx::DocumentPtr doc, mx::ConstDocumentPtr stdlib)
{
    for (auto elem : doc->traverseTree())
    {
        auto port = elem->asA<mx::PortElement>();
        if (!port)
            continue;

        const mx::StringVec& attributeNames = port->getAttributeNames();
        for (const std::string& attrName : attributeNames) {
            std::string valueStr = port->getAttribute(attrName);
            port->setAttribute(attrName, replaceConstantValues(valueStr, port->getType(), stdlib));
        }
    }
}

void removeTypeDefConstants(mx::DocumentPtr doc)
{
    auto typedefElems = doc->getChildrenOfType<mx::TypeDef>();
    for (mx::TypeDefPtr typedefElem : typedefElems)
    {
        auto constantElems = typedefElem->getChildrenOfType<mx::Element>("constant");
        for (const auto& constantElem : constantElems)
        {
            typedefElem->removeChild(constantElem->getName());
        }
    }
}

int main(int argc, char* const argv[])
{
    std::vector<std::string> tokens;
    for (int i = 1; i < argc; i++)
    {
        tokens.emplace_back(argv[i]);
    }

    std::string sourceLibraryRootStr = "";
    std::string destLibraryRootStr = "";
    bool bakeNamedValues = false;
    bool expandTemplateElems = false;

    for (size_t i = 0; i < tokens.size(); i++)
    {
        const std::string& token = tokens[i];
        const std::string& nextToken = i + 1 < tokens.size() ? tokens[i + 1] : mx::EMPTY_STRING;

        if (token == "--sourceLibraryRoot")
        {
            sourceLibraryRootStr = nextToken;
        }
        else if (token == "--destLibraryRoot")
        {
            destLibraryRootStr = nextToken;
        }
        else if (token == "--bakeNamedValues")
        {
            bakeNamedValues = true;
            continue;
        }
        else if (token == "--expandTemplateElems")
        {
            expandTemplateElems = true;
            continue;
        }
        else if (token == "--help")
        {
            std::cout << " MaterialXBuildLibrary version " << mx::getVersionString() << std::endl;
            std::cout << cmdLineOptions << std::endl;
            return 0;
        }
        else
        {
            std::cout << "Unrecognized command-line option: " << token << std::endl;
            std::cout << "Launch the viewer with '--help' for a complete list of supported options." << std::endl;
            continue;
        }

        if (nextToken.empty())
        {
            std::cout << "Expected another token following command-line option: " << token << std::endl;
        }
        else
        {
            i++;
        }
    }

    mx::FilePath sourceLibraryRoot = mx::FilePath(sourceLibraryRootStr);
    mx::FilePath destLibrarayRoot = mx::FilePath(destLibraryRootStr);

    if (!sourceLibraryRoot.isDirectory())
    {
        std::cerr << "Source Library Root is not a directory" << std::endl;
        return 1;
    }

    if (!destLibrarayRoot.isDirectory())
    {
        std::cerr << "Destination Library Root is not a directory" << std::endl;
        return 1;
    }

    mx::DocumentPtr stdlib = mx::createDocument();
    mx::loadLibraries({}, mx::FileSearchPath(sourceLibraryRootStr), stdlib);

    mx::FilePathVec mtlxFiles = getMaterialXFiles(sourceLibraryRoot);

    mx::XmlReadOptions readOptions;
    readOptions.readComments = true;
    readOptions.readNewlines = true;
    readOptions.expandTemplateElems = expandTemplateElems;

    mx::XmlWriteOptions writeOptions;
    writeOptions.createDirectories = true;

    for (const auto& mtlxFile : mtlxFiles)
    {
        mx::DocumentPtr doc = mx::createDocument();

        mx::FilePath sourceFile = sourceLibraryRoot / mtlxFile;
        mx::FilePath destFile = destLibrarayRoot / mtlxFile;

        mx::readFromXmlFile(doc, sourceFile, mx::FileSearchPath(), &readOptions);

        if (bakeNamedValues)
        {
            replaceConstantValues(doc, stdlib);
            removeTypeDefConstants(doc);
        }

        {
            // remove "specification" attributes all start with "spec_"
            for (auto elem: doc->traverseTree())
            {
                mx::StringVec attributeNames = elem->getAttributeNames();
                for (const auto& attrName: attributeNames) {
                    if (mx::stringStartsWith(attrName, "spec_")) {
                        elem->removeAttribute(attrName);
                    }
                }
            }
        }

        mx::writeToXmlFile(doc, destFile, &writeOptions);
    }

    return 0;
}
