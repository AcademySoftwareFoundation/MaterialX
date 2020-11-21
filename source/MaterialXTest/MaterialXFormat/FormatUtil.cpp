//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Util.h>
#include <MaterialXCore/Document.h>
#include <MaterialXFormat/Util.h>

namespace mx = MaterialX;

TEST_CASE("resolveFileNames", "[formatutil]")
{
    const mx::FilePath TEST_FILE_PREFIX_STRING("resources\\Images\\");
    const mx::FilePath TEST_IMAGE_STRING1("brass_roughness.jpg");
    const mx::FilePath TEST_IMAGE_STRING2("brass_color.jpg");

    mx::DocumentPtr doc1 = mx::createDocument();

    // Set up document
    mx::NodeGraphPtr nodeGraph = doc1->addNodeGraph();
    nodeGraph->setFilePrefix(TEST_FILE_PREFIX_STRING.asString() + "\\"); // Note this is required as filepath->string strips out last separator
    mx::NodePtr image1 = nodeGraph->addNode("image");
    image1->setInputValue("file", "brass_roughness.jpg", mx::FILENAME_TYPE_STRING);
    mx::NodePtr image2 = nodeGraph->addNode("image");
    image2->setInputValue("file", "brass_color.jpg", mx::FILENAME_TYPE_STRING);

    // 1. Test resolving fileprefix
    mx::resolveFileNames(doc1);
    REQUIRE(nodeGraph->getFilePrefix() == mx::EMPTY_STRING);
    mx::FilePath resolvedPath(image1->getInputValue("file")->getValueString());
    REQUIRE(resolvedPath == (TEST_FILE_PREFIX_STRING / TEST_IMAGE_STRING1));
    resolvedPath = image2->getInputValue("file")->getValueString();
    REQUIRE(resolvedPath == (TEST_FILE_PREFIX_STRING / TEST_IMAGE_STRING2));

    // Reset document
    nodeGraph->setFilePrefix(TEST_FILE_PREFIX_STRING.asString() + "\\");
    image1->setInputValue("file", "brass_roughness.jpg", mx::FILENAME_TYPE_STRING);
    image2->setInputValue("file", "brass_color.jpg", mx::FILENAME_TYPE_STRING);

    // 2. Test resolving to absolute paths
    mx::FilePath rootPath(mx::FilePath::getCurrentPath());

    mx::FileSearchPath searchPath;
    searchPath.append(rootPath);

    mx::resolveFileNames(doc1, searchPath);    
    CHECK(nodeGraph->getFilePrefix() == mx::EMPTY_STRING);
    resolvedPath = image1->getInputValue("file")->getValueString();
    CHECK(resolvedPath.asString() == (rootPath / TEST_FILE_PREFIX_STRING / TEST_IMAGE_STRING1).asString());
    resolvedPath = image2->getInputValue("file")->getValueString();
    CHECK(resolvedPath.asString() == (rootPath / TEST_FILE_PREFIX_STRING / TEST_IMAGE_STRING2).asString());

    // Reset document
    nodeGraph->setFilePrefix(TEST_FILE_PREFIX_STRING.asString() + "\\");
    image1->setInputValue("file", "brass_roughness.jpg", mx::FILENAME_TYPE_STRING);
    image2->setInputValue("file", "brass_color.jpg", mx::FILENAME_TYPE_STRING);

    // 3. Test with additional resolvers
    // - Create resolver to replace all Windows separators with POSIX ones
    mx::StringResolverPtr separatorReplacer = mx::StringResolver::create();
    separatorReplacer->setFilenameSubstitution("\\\\", "/");
    separatorReplacer->setFilenameSubstitution("\\", "/");

    mx::resolveFileNames(doc1, searchPath, separatorReplacer);
    CHECK(nodeGraph->getFilePrefix() == mx::EMPTY_STRING);
    std::string resolvedPathString = image1->getInputValue("file")->getValueString();
    CHECK(resolvedPathString == (rootPath / TEST_FILE_PREFIX_STRING / TEST_IMAGE_STRING1).asString(mx::FilePath::FormatPosix));
    resolvedPathString = image2->getInputValue("file")->getValueString();
    CHECK(resolvedPathString == (rootPath / TEST_FILE_PREFIX_STRING / TEST_IMAGE_STRING2).asString(mx::FilePath::FormatPosix));

    // 4. Test with pre-resolved filenames
    nodeGraph->setFilePrefix(TEST_FILE_PREFIX_STRING.asString() + "\\");
    mx::resolveFileNames(doc1, searchPath, separatorReplacer);
    CHECK(nodeGraph->getFilePrefix() == mx::EMPTY_STRING);
    resolvedPathString = image1->getInputValue("file")->getValueString();
    CHECK(resolvedPathString == (rootPath / TEST_FILE_PREFIX_STRING / TEST_IMAGE_STRING1).asString(mx::FilePath::FormatPosix));
    resolvedPathString = image2->getInputValue("file")->getValueString();
    CHECK(resolvedPathString == (rootPath / TEST_FILE_PREFIX_STRING / TEST_IMAGE_STRING2).asString(mx::FilePath::FormatPosix));
}
