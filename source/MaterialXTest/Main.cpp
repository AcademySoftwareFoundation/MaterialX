//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#define CATCH_CONFIG_RUNNER
#include <MaterialXTest/Catch/catch.hpp>

int main(int argc, char* const argv[])
{
    Catch::Session session;

#ifndef _DEBUG
    session.configData().showDurations = Catch::ShowDurations::Always;
#endif
#ifdef CATCH_PLATFORM_WINDOWS
//    session.configData().outputFilename = "%debug";
#endif

    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0)
    {
        return returnCode;
    }

    return session.run();
}
