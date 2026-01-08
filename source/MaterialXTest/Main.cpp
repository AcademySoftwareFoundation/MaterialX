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
#include <chrono>
#include <iomanip>
#include <sstream>
#endif

namespace mx = MaterialX;

#ifdef MATERIALX_BUILD_TRACING
// Generate timestamp string for trace filename (format: YYYYMMDD_HHMMSS)
std::string getTimestampString()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &time);
#else
    localtime_r(&time, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y%m%d_%H%M%S");
    return oss.str();
}
#endif

int main(int argc, char* const argv[])
{
    Catch::Session session;
    session.configData().showDurations = Catch::ShowDurations::Always;

#ifdef MATERIALX_BUILD_TRACING
    // Capture start timestamp for trace filename
    std::string traceTimestamp = getTimestampString();
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

    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0)
    {
        return returnCode;
    }

    // Check if we're in list/discovery mode (used by test adapters)
    bool isListMode = false;
    for (int i = 1; i < argc; ++i)
    {
        std::string arg(argv[i]);
        if (arg.find("--list") == 0)
        {
            isListMode = true;
            break;
        }
    }

#ifdef MATERIALX_BUILD_TRACING
    std::shared_ptr<mx::MxPerfettoBackend> perfettoBackend;
    // Skip tracing initialization during test discovery (--list-* commands)
    if (!isListMode)
    {
        perfettoBackend = mx::MxPerfettoBackend::create();
        perfettoBackend->initialize();
        mx::MxTraceCollector::getInstance().setBackend(perfettoBackend);
    }
#endif

    int result = session.run();

#ifdef MATERIALX_BUILD_TRACING
    // Shutdown tracing and write trace file with timestamp (skip if in list mode)
    if (!isListMode && perfettoBackend)
    {
        mx::MxTraceCollector::getInstance().setBackend(nullptr);
        std::string traceFilename = "materialx_test_" + traceTimestamp + ".perfetto-trace";
        perfettoBackend->shutdown(traceFilename);
    }
#endif

    return result;
}
