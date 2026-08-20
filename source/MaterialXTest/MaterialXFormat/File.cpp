//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

#include <MaterialXCore/Exception.h>

#include <fstream>

#if !defined(_WIN32)
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace mx = MaterialX;

TEST_CASE("Syntactic operations", "[file]")
{
    using InputPair = std::pair<std::string, mx::FilePath::Format>;
    std::vector<InputPair> inputPairs =
    {
        {"D:\\Assets\\Materials\\Robot.mtlx", mx::FilePath::FormatWindows},
        {"\\\\Show\\Assets\\Materials\\Robot.mtlx", mx::FilePath::FormatWindows},
        {"Materials\\Robot.mtlx", mx::FilePath::FormatWindows},
        {"/Assets/Materials/Robot.mtlx", mx::FilePath::FormatPosix},
        {"Assets/Materials/Robot.mtlx", mx::FilePath::FormatPosix},
        {"Materials/Robot.mtlx", mx::FilePath::FormatPosix}
    };

    for (const InputPair& pair : inputPairs)
    {
        mx::FilePath path(pair.first);
        REQUIRE(path.asString(pair.second) == pair.first);
    }
}

TEST_CASE("File system operations", "[file]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePathVec examplePaths =
    {
        "libraries/stdlib/stdlib_defs.mtlx",
        "resources/Materials/Examples/StandardSurface/standard_surface_brass_tiled.mtlx",
        "resources/Materials/Examples/StandardSurface/standard_surface_marble_solid.mtlx",
    };
    for (const mx::FilePath& path : examplePaths)
    {
        REQUIRE(searchPath.find(path).exists());
    }

    REQUIRE(mx::FilePath::getCurrentPath().exists());
    REQUIRE(mx::FilePath::getModulePath().exists());
}

TEST_CASE("File search path operations", "[file]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    searchPath.append(searchPath.find("libraries/stdlib"));
    searchPath.append(searchPath.find("resources/Materials/Examples/StandardSurface"));

    mx::FilePathVec filenames =
    {
        "stdlib_defs.mtlx",
        "standard_surface_brass_tiled.mtlx",
        "standard_surface_marble_solid.mtlx",
    };

    for (const mx::FilePath& filename : filenames)
    {
        REQUIRE(searchPath.find(filename).exists());
    }
}

TEST_CASE("Flatten filenames", "[file]")
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
    mx::flattenFilenames(doc1);
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
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath rootPath = searchPath.isEmpty() ? mx::FilePath() : searchPath[0];

    mx::flattenFilenames(doc1, searchPath);    
    REQUIRE(nodeGraph->getFilePrefix() == mx::EMPTY_STRING);
    resolvedPath = image1->getInputValue("file")->getValueString();
    REQUIRE(resolvedPath.asString() == (rootPath / TEST_FILE_PREFIX_STRING / TEST_IMAGE_STRING1).asString());
    resolvedPath = image2->getInputValue("file")->getValueString();
    REQUIRE(resolvedPath.asString() == (rootPath / TEST_FILE_PREFIX_STRING / TEST_IMAGE_STRING2).asString());

    // Reset document
    nodeGraph->setFilePrefix(TEST_FILE_PREFIX_STRING.asString() + "\\");
    image1->setInputValue("file", "brass_roughness.jpg", mx::FILENAME_TYPE_STRING);
    image2->setInputValue("file", "brass_color.jpg", mx::FILENAME_TYPE_STRING);

    // 3. Test with additional resolvers
    // - Create resolver to replace all Windows separators with POSIX ones
    mx::StringResolverPtr separatorReplacer = mx::StringResolver::create();
    separatorReplacer->setFilenameSubstitution("\\\\", "/");
    separatorReplacer->setFilenameSubstitution("\\", "/");

    mx::flattenFilenames(doc1, searchPath, separatorReplacer);
    REQUIRE(nodeGraph->getFilePrefix() == mx::EMPTY_STRING);
    std::string resolvedPathString = image1->getInputValue("file")->getValueString();
    REQUIRE(resolvedPathString == (rootPath / TEST_FILE_PREFIX_STRING / TEST_IMAGE_STRING1).asString(mx::FilePath::FormatPosix));
    resolvedPathString = image2->getInputValue("file")->getValueString();
    REQUIRE(resolvedPathString == (rootPath / TEST_FILE_PREFIX_STRING / TEST_IMAGE_STRING2).asString(mx::FilePath::FormatPosix));

    // 4. Test with pre-resolved filenames
    nodeGraph->setFilePrefix(TEST_FILE_PREFIX_STRING.asString() + "\\");
    mx::flattenFilenames(doc1, searchPath, separatorReplacer);
    REQUIRE(nodeGraph->getFilePrefix() == mx::EMPTY_STRING);
    resolvedPathString = image1->getInputValue("file")->getValueString();
    REQUIRE(resolvedPathString == (rootPath / TEST_FILE_PREFIX_STRING / TEST_IMAGE_STRING1).asString(mx::FilePath::FormatPosix));
    resolvedPathString = image2->getInputValue("file")->getValueString();
    REQUIRE(resolvedPathString == (rootPath / TEST_FILE_PREFIX_STRING / TEST_IMAGE_STRING2).asString(mx::FilePath::FormatPosix));
}

TEST_CASE("Path normalization test", "[file]")
{
    const mx::FilePath REFERENCE_REL_PATH("a/b");
    const mx::FilePath REFERENCE_ABS_PREFIX("/assets");

    std::vector<mx::FilePath> examplePaths =
    {
        "a/./b",
        "././a/b",
        "c/../d/../a/b",
        "a/b/./c/d/../.."
    };

    for (const mx::FilePath& path : examplePaths)
    {
        REQUIRE(path.getNormalized() == REFERENCE_REL_PATH);
        REQUIRE((REFERENCE_ABS_PREFIX / path).getNormalized() == (REFERENCE_ABS_PREFIX / REFERENCE_REL_PATH));
    }
}

TEST_CASE("Get all files in directory", "[file]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();

    mx::FilePath lightsDir = searchPath.find("resources/Lights");

    mx::FilePathVec mtlxFilenames =
    {
        "environment_map.mtlx",
        "goegap_split.mtlx",
        "san_giuseppe_bridge_split.mtlx",
        "table_mountain_split.mtlx"
    };

    mx::FilePathVec hdrFilenames =
    {
        "goegap.hdr",
        "goegap_split.hdr",
        "san_giuseppe_bridge.hdr",
        "san_giuseppe_bridge_split.hdr",
        "table_mountain.hdr",
        "table_mountain_split.hdr",
    };

    mx::FilePathVec allFilesnames = hdrFilenames;
    allFilesnames.insert(allFilesnames.begin(), mtlxFilenames.begin(), mtlxFilenames.end());

    mx::FilePathVec results;

    results = lightsDir.getFilesInDirectory("mtlx");
    REQUIRE(results.size() > 0);
    for (const mx::FilePath& filename : mtlxFilenames)
    {
        REQUIRE(std::find(results.begin(), results.end(), filename) != results.end());
    }

    results = lightsDir.getFilesInDirectory("hdr");
    REQUIRE(results.size() > 0);
    for (const mx::FilePath& filename : hdrFilenames)
    {
        REQUIRE(std::find(results.begin(), results.end(), filename) != results.end());
    }

    results = lightsDir.getFilesInDirectory();
    REQUIRE(results.size() > 0);
    for (const mx::FilePath& filename : allFilesnames)
    {
        REQUIRE(std::find(results.begin(), results.end(), filename) != results.end());
    }
}

TEST_CASE("System temporary directory", "[file]")
{
    mx::FilePath systemTempDir = mx::FilePath::getSystemTemporaryDirectory();

    REQUIRE(!systemTempDir.isEmpty());
    REQUIRE(systemTempDir.exists());
    REQUIRE(systemTempDir.isDirectory());
}

TEST_CASE("Create temporary directory", "[file]")
{
    // Create a temporary directory within the system temporary directory.
    mx::FilePath systemTempDir = mx::FilePath::getSystemTemporaryDirectory();
    mx::FilePath tempDir = mx::FilePath::createTemporaryDirectory();
    REQUIRE(tempDir.exists());
    REQUIRE(tempDir.isDirectory());
    REQUIRE(tempDir.getParentPath().getNormalized() == systemTempDir.getNormalized());

    // Verify that each temporary directory is unique.
    mx::FilePath secondTempDir = mx::FilePath::createTemporaryDirectory();
    REQUIRE(secondTempDir.isDirectory());
    REQUIRE(secondTempDir != tempDir);

    // Create a temporary directory within an explicit parent directory, which
    // is itself created on demand.
    mx::FilePath explicitParent = tempDir / "parent" / "subdirectory";
    REQUIRE(!explicitParent.exists());
    mx::FilePath childTempDir = mx::FilePath::createTemporaryDirectory(explicitParent);
    REQUIRE(childTempDir.isDirectory());
    REQUIRE(childTempDir.getParentPath().getNormalized() == explicitParent.getNormalized());

    // Verify that a parent path referring to an existing file is rejected.
    mx::FilePath existingFile = tempDir / "file.txt";
    std::ofstream(existingFile.asString()) << "content";
    REQUIRE(existingFile.exists());
    REQUIRE(!existingFile.isDirectory());
    REQUIRE_THROWS_AS(mx::FilePath::createTemporaryDirectory(existingFile), mx::Exception);

#if !defined(_WIN32)
    // Verify that the temporary directory is accessible only to the current user.
    struct stat sb;
    REQUIRE(stat(tempDir.asString().c_str(), &sb) == 0);
    REQUIRE((sb.st_mode & 07777) == 0700);

    // Verify that a failure to create the directory is reported as an exception,
    // rather than returning a path that does not exist.
    mx::FilePath readOnlyParent = tempDir / "readOnly";
    readOnlyParent.createDirectory();
    REQUIRE(readOnlyParent.isDirectory());
    REQUIRE(chmod(readOnlyParent.asString().c_str(), 0500) == 0);
    REQUIRE_THROWS_AS(mx::FilePath::createTemporaryDirectory(readOnlyParent), mx::Exception);
    REQUIRE(chmod(readOnlyParent.asString().c_str(), 0700) == 0);
#endif

    REQUIRE(tempDir.removeDirectory(true));
    REQUIRE(!tempDir.exists());
    REQUIRE(secondTempDir.removeDirectory(true));
    REQUIRE(!secondTempDir.exists());
}

TEST_CASE("Remove directory", "[file]")
{
    mx::FilePath tempDir = mx::FilePath::createTemporaryDirectory();

    // Verify that a non-recursive removal leaves a non-empty directory in place.
    mx::FilePath nestedDir = tempDir / "nested" / "subdirectory";
    nestedDir.createDirectory(true);
    std::ofstream((nestedDir / "file.txt").asString()) << "content";
    REQUIRE(!tempDir.removeDirectory());
    REQUIRE(tempDir.isDirectory());

    // Verify that a non-recursive removal succeeds for an empty directory.
    mx::FilePath emptyDir = tempDir / "empty";
    emptyDir.createDirectory();
    REQUIRE(emptyDir.removeDirectory());
    REQUIRE(!emptyDir.exists());

    // Verify that a removal of a path that does not exist is reported as a failure.
    REQUIRE(!emptyDir.removeDirectory());
    REQUIRE(!emptyDir.removeDirectory(true));

#if !defined(_WIN32)
    // Verify that a symbolic link within the directory is removed without following
    // it to its target.
    mx::FilePath linkTarget = mx::FilePath::createTemporaryDirectory();
    std::ofstream((linkTarget / "preserved.txt").asString()) << "content";
    mx::FilePath link = nestedDir / "link";
    REQUIRE(symlink(linkTarget.asString().c_str(), link.asString().c_str()) == 0);
    REQUIRE(link.isDirectory());
#endif

    // Verify that a recursive removal clears the directory and its contents.
    REQUIRE(tempDir.removeDirectory(true));
    REQUIRE(!tempDir.exists());

#if !defined(_WIN32)
    REQUIRE(linkTarget.isDirectory());
    REQUIRE((linkTarget / "preserved.txt").exists());
    REQUIRE(linkTarget.removeDirectory(true));
#endif
}
