//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/MaterialXRender/RenderJobs.h>

#include <MaterialXRender/ShaderRenderer.h>
#include <MaterialXRender/Timer.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>

#if defined(__linux__)
#include <sched.h>
#endif

namespace RenderUtil
{

RenderJobDispatcher::RenderJobDispatcher(unsigned int workerCount) :
    _workerCount(workerCount)
{
    if (!workerCount)
        throw std::invalid_argument("Render worker count must be positive");
    try
    {
        if (workerCount > 1)
        {
            for (unsigned int i = 0; i < workerCount; ++i)
                _threads.emplace_back([this] { run(); });
        }
    }
    catch (...)
    {
        close();
        throw;
    }
}

RenderJobDispatcher::~RenderJobDispatcher()
{
    close();
}

void RenderJobDispatcher::submit(const std::string& label,
                                 const MaterialX::FilePath& outputPrefix, Work work)
{
    if (_closed)
        throw std::logic_error("Cannot submit to a closed render dispatcher");
    auto path = std::filesystem::weakly_canonical(std::filesystem::absolute(outputPrefix.asString()));
    auto key = path.generic_string();
#if defined(_WIN32)
    std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
#endif
    if (!_outputPaths.insert(key).second)
        throw std::runtime_error("Conflicting render output: " + key + " (" + label + ")");

    std::packaged_task<RenderJobResult()> task([label, work = std::move(work)] {
        RenderJobResult result;
        result.label = label;
        MaterialX::ScopedTimer timer(&result.totalTime);
        try
        {
            work(result);
        }
        catch (MaterialX::ExceptionRenderError& e)
        {
            result.success = false;
            result.log += std::string(e.what()) + "\n";
            for (const auto& error : e.errorLog())
                result.log += error + "\n";
        }
        catch (std::exception& e)
        {
            result.success = false;
            result.log += std::string(e.what()) + "\n";
        }
        catch (...)
        {
            result.success = false;
            result.log += "Unknown exception in render job\n";
        }
        timer.endTimer();
        return result;
    });
    _results.push_back(task.get_future());
    if (_workerCount == 1)
    {
        task();
        return;
    }

    std::unique_lock<std::mutex> lock(_mutex);
    _space.wait(lock, [this] { return _queue.size() < size_t(_workerCount) * 2; });
    _queue.push_back(std::move(task));
    lock.unlock();
    _ready.notify_one();
}

void RenderJobDispatcher::run()
{
    for (;;)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _ready.wait(lock, [this] { return _closed || !_queue.empty(); });
        if (_queue.empty())
            return;
        auto task = std::move(_queue.front());
        _queue.pop_front();
        lock.unlock();
        _space.notify_one();
        task();
    }
}

void RenderJobDispatcher::close()
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _closed = true;
    }
    _ready.notify_all();
    for (auto& thread : _threads)
        if (thread.joinable())
            thread.join();
}

std::vector<RenderJobResult> RenderJobDispatcher::finish()
{
    close();
    std::vector<RenderJobResult> results;
    for (auto& future : _results)
        results.push_back(future.get());
    _results.clear();
    return results;
}

unsigned int availableCpuCount()
{
#if defined(__linux__)
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    if (sched_getaffinity(0, sizeof(cpus), &cpus) == 0 && CPU_COUNT(&cpus) > 0)
        return static_cast<unsigned int>(CPU_COUNT(&cpus));
#endif
    return std::max(1u, std::thread::hardware_concurrency());
}

unsigned int oslThreadCount(int workers, int threadsPerWorker, unsigned int cpuCount)
{
    if (workers < 1 || threadsPerWorker < 0)
        throw std::invalid_argument("Invalid OSL worker or thread count");
    if (threadsPerWorker > 0)
        return static_cast<unsigned int>(threadsPerWorker);
    return workers == 1 ? 0 : std::max(1u, cpuCount / static_cast<unsigned int>(workers));
}

} // namespace RenderUtil
