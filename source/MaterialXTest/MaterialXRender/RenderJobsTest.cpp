//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXTest/MaterialXRender/RenderJobs.h>
#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>

#include <MaterialXRender/Util.h>
#include <MaterialXRender/ShaderRenderer.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>

namespace
{
struct TestDirectory
{
    TestDirectory()
    {
        static std::atomic<unsigned int> count { 0 };
        path = std::filesystem::temp_directory_path() /
            ("materialx render jobs " + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) +
             "-" + std::to_string(count++));
        std::filesystem::create_directories(path);
    }
    ~TestDirectory()
    {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
    MaterialX::FilePath file(const std::string& name) { return (path / name).string(); }
    std::filesystem::path path;
};
}

TEST_CASE("Render: Worker scheduling", "[renderworkers]")
{
    TestDirectory dir;
    RenderUtil::RenderJobDispatcher jobs(2);
    std::promise<void> release;
    auto gate = release.get_future().share();
    std::promise<void> laterFinished;
    auto later = laterFinished.get_future();
    std::atomic<int> active { 0 }, peak { 0 }, executed { 0 };
    auto enter = [&] {
        int n = ++active;
        int old = peak.load();
        while (old < n && !peak.compare_exchange_weak(old, n)) { }
    };
    jobs.submit("blocked", dir.file("0"), [&](RenderUtil::RenderJobResult&) {
        enter();
        gate.wait();
        --active;
        ++executed;
    });
    for (int i = 1; i < 8; ++i)
    {
        jobs.submit(std::to_string(i), dir.file(std::to_string(i)), [&, i](RenderUtil::RenderJobResult&) {
            enter();
            --active;
            ++executed;
            if (i == 7)
                laterFinished.set_value();
        });
    }
    // A free worker must keep draining jobs while an earlier job is blocked.
    auto status = later.wait_for(std::chrono::seconds(5));
    release.set_value();
    auto results = jobs.finish();
    CHECK(status == std::future_status::ready);
    CHECK(executed == 8);
    CHECK(peak <= 2);
    REQUIRE(results.size() == 8);
    CHECK(results[0].label == "blocked");
    for (int i = 1; i < 8; ++i)
        CHECK(results[i].label == std::to_string(i));
    CHECK_THROWS(jobs.submit("closed", dir.file("closed"), [](RenderUtil::RenderJobResult&) { }));
}

TEST_CASE("Render: Worker queue backpressure", "[renderworkers]")
{
    TestDirectory dir;
    RenderUtil::RenderJobDispatcher jobs(2);
    std::promise<void> release;
    auto gate = release.get_future().share();
    std::atomic<int> entered { 0 };
    auto work = [&](RenderUtil::RenderJobResult&) { ++entered; gate.wait(); };
    jobs.submit("0", dir.file("0"), work);
    jobs.submit("1", dir.file("1"), work);
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (entered < 2 && std::chrono::steady_clock::now() < deadline)
        std::this_thread::yield();
    // Two running plus four queued fills a two-worker dispatcher.
    for (int i = 2; i < 6; ++i)
        jobs.submit(std::to_string(i), dir.file(std::to_string(i)), work);
    auto blocked = std::async(std::launch::async, [&] { jobs.submit("6", dir.file("6"), work); });
    auto status = blocked.wait_for(std::chrono::milliseconds(30));
    release.set_value();
    blocked.get();
    CHECK(status == std::future_status::timeout);
    CHECK(jobs.finish().size() == 7);
    CHECK(entered == 7);
}

TEST_CASE("Render: Worker failures and cleanup", "[renderworkers]")
{
    TestDirectory dir;
    for (unsigned int workers : { 1u, 3u })
    {
        RenderUtil::RenderJobDispatcher jobs(workers);
        auto caller = std::this_thread::get_id();
        jobs.submit("exception", dir.file("exception"), [](RenderUtil::RenderJobResult&) {
            throw MaterialX::ExceptionRenderError("compiler failed", { "compiler details" });
        });
        jobs.submit("unknown", dir.file("unknown"), [](RenderUtil::RenderJobResult&) { throw 7; });
        jobs.submit("success", dir.file("success"), [caller, workers](RenderUtil::RenderJobResult& r) {
            r.success = workers != 1 || caller == std::this_thread::get_id();
        });
        CHECK_THROWS(jobs.submit("collision", dir.file("./success"), [](RenderUtil::RenderJobResult&) { }));
        auto results = jobs.finish();
        REQUIRE(results.size() == 3);
        CHECK_FALSE(results[0].success);
        CHECK(results[0].log.find("compiler details") != std::string::npos);
        CHECK_FALSE(results[1].success);
        CHECK(results[2].success);
    }
    std::atomic<int> completed { 0 };
    {
        RenderUtil::RenderJobDispatcher jobs(3);
        for (int i = 0; i < 12; ++i)
            jobs.submit(std::to_string(i), dir.file(std::to_string(i)), [&](RenderUtil::RenderJobResult&) { ++completed; });
    }
    CHECK(completed == 12);
    RenderUtil::RenderJobDispatcher empty(2);
    CHECK(empty.finish().empty());
    CHECK_THROWS(RenderUtil::RenderJobDispatcher(0));
}

TEST_CASE("Render: Worker options", "[renderworkers]")
{
    TestDirectory dir;
    auto optionsPath = dir.file("options.mtlx");
    auto writeOptions = [&](const std::string& inputs) {
        std::ofstream file(optionsPath.asString());
        file << "<materialx version=\"1.39\"><nodedef name=\"TestSuiteOptions\">" << inputs << "</nodedef></materialx>";
    };
    GenShaderUtil::TestSuiteOptions options;
    writeOptions("");
    REQUIRE(options.readOptions(optionsPath));
    CHECK(options.oslRenderWorkers == 1);
    CHECK(options.mdlRenderWorkers == 1);
    CHECK(options.oslThreadsPerWorker == 0);
    writeOptions("<input name=\"oslRenderWorkers\" type=\"integer\" value=\"8\"/>"
                 "<input name=\"mdlRenderWorkers\" type=\"integer\" value=\"12\"/>"
                 "<input name=\"oslThreadsPerWorker\" type=\"integer\" value=\"3\"/>");
    REQUIRE(options.readOptions(optionsPath));
    CHECK(options.oslRenderWorkers == 8);
    CHECK(options.mdlRenderWorkers == 12);
    CHECK(options.oslThreadsPerWorker == 3);
    for (auto name : { "oslRenderWorkers", "mdlRenderWorkers", "oslThreadsPerWorker" })
    {
        writeOptions("<input name=\"" + std::string(name) + "\" type=\"integer\" value=\"-1\"/>");
        CHECK_FALSE(options.readOptions(optionsPath));
    }
    writeOptions("<input name=\"oslRenderWorkers\" type=\"integer\" value=\"0\"/>");
    CHECK_FALSE(options.readOptions(optionsPath));
    writeOptions("<input name=\"mdlRenderWorkers\" type=\"string\" value=\"eight\"/>");
    CHECK_FALSE(options.readOptions(optionsPath));
    CHECK(RenderUtil::oslThreadCount(1, 0, 24) == 0);
    CHECK(RenderUtil::oslThreadCount(8, 0, 24) == 3);
    CHECK(RenderUtil::oslThreadCount(12, 0, 24) == 2);
    CHECK(RenderUtil::oslThreadCount(8, 0, 2) == 1);
    CHECK(RenderUtil::oslThreadCount(8, 0, 0) == 1);
    CHECK(RenderUtil::oslThreadCount(8, 5, 24) == 5);
    CHECK(RenderUtil::availableCpuCount() > 0);
}

TEST_CASE("Render: Child working directories", "[renderworkers]")
{
    TestDirectory dir;
    auto original = std::filesystem::current_path();
    RenderUtil::RenderJobDispatcher jobs(2);
    for (auto name : { "first directory", "second directory" })
    {
        auto child = dir.path / name;
        std::filesystem::create_directory(child);
        std::ofstream(child / "marker.txt") << name;
        auto output = child / "output.txt";
#if defined(_WIN32)
        std::string command = "type marker.txt > ";
#else
        std::string command = "cat marker.txt > ";
#endif
        command += MaterialX::quoteCommandArgument(output.string());
        command = MaterialX::commandInDirectory(command, child.string());
        jobs.submit(name, output.string(), [command](RenderUtil::RenderJobResult& r) {
            r.success = std::system(command.c_str()) == 0;
        });
    }
    for (auto& result : jobs.finish())
    {
        CHECK(result.success);
        std::ifstream output(dir.path / result.label / "output.txt");
        std::string contents;
        std::getline(output, contents);
        CHECK(contents == result.label);
    }
    CHECK(std::filesystem::current_path() == original);
#if !defined(_WIN32)
    auto output = dir.path / "quotes' and $dollars; space.txt";
    auto command = "printf '%s' " + MaterialX::quoteCommandArgument("quotes' and $dollars;") +
                   " > " + MaterialX::quoteCommandArgument(output.string());
    REQUIRE(std::system(command.c_str()) == 0);
    std::ifstream stream(output);
    std::string contents;
    std::getline(stream, contents);
    CHECK(contents == "quotes' and $dollars;");
#endif
}
