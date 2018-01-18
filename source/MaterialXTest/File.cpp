//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXFormat/File.h>

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
        mx::FilePath path;
        path.assign(pair.first, pair.second);
        REQUIRE(path.asString(pair.second) == pair.first);
    }
}

TEST_CASE("File system operations", "[file]")
{
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Examples/MaterialGraphs.mtlx",
        "documents/Examples/PaintMaterials.mtlx",
    };

    for (const std::string& filename : filenames)
    {
        mx::FilePath path(filename);
        REQUIRE(path.exists());
        REQUIRE(mx::FileSearchPath().find(path).exists());
    }
}
