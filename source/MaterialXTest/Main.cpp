//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#define CATCH_CONFIG_RUNNER

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXFormat/File.h>

#ifdef MATERIALX_BUILD_TRACING
#include <MaterialXCore/MxTrace.h>
#include <MaterialXCore/MxTracePerfetto.h>
#endif

namespace mx = MaterialX;

int main(int argc, char* const argv[])
{
    Catch::Session session;
    session.configData().showDurations = Catch::ShowDurations::Always;

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

    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0)
    {
        return returnCode;
    }

#ifdef MATERIALX_BUILD_TRACING
    // Initialize Perfetto tracing
    auto perfettoBackend = mx::MxPerfettoBackend::create();
    perfettoBackend->initialize();
    mx::MxTraceCollector::getInstance().setBackend(perfettoBackend);
#endif

    int result = session.run();

#ifdef MATERIALX_BUILD_TRACING
    // Shutdown tracing and write trace file
    mx::MxTraceCollector::getInstance().setBackend(nullptr);
    perfettoBackend->shutdown("materialx_test_trace.perfetto-trace");
#endif

    return result;
}
