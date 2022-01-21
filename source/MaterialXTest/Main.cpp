//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#define CATCH_CONFIG_RUNNER

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXFormat/File.h>

namespace mx = MaterialX;

int main(int argc, char* const argv[])
{
    Catch::Session session;

#ifndef _DEBUG
    session.configData().showDurations = Catch::ShowDurations::Always;
#endif
#ifdef CATCH_PLATFORM_WINDOWS
    BOOL inDebugger = IsDebuggerPresent();
    if (inDebugger)
    {
        session.configData().outputFilename = "%debug";
    }
    else
    {
        session.configData().outputFilename = "";
    }
#endif

    // If the current path has no valid resources folder, as can occur when launching the
    // test suite from an IDE, then align the current path with the module path.
    mx::FilePath resourcesPath = mx::FilePath::getCurrentPath() / "resources";
    if (!resourcesPath.exists())
    {
        resourcesPath = mx::FilePath::getModulePath().getParentPath() / "resources";
        if (resourcesPath.exists())
        {
            resourcesPath.getParentPath().setCurrentPath();
        }
    }

    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0)
    {
        return returnCode;
    }

    return session.run();
}
