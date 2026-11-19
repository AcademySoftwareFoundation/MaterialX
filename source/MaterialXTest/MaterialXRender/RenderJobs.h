//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_TEST_RENDER_JOBS_H
#define MATERIALX_TEST_RENDER_JOBS_H

#include <MaterialXFormat/File.h>

#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <set>
#include <thread>

namespace RenderUtil
{

struct RenderJobResult
{
    std::string label;
    std::string log;
    MaterialX::FilePath imagePath;
    bool success = true;
    double totalTime = 0.0;
    double compileTime = 0.0;
    double renderTime = 0.0;
};

// Submission and result collection belong to the test thread. Jobs own all
// their inputs and must not access Catch, documents, or shared log streams.
class RenderJobDispatcher
{
  public:
    using Work = std::function<void(RenderJobResult&)>;

    explicit RenderJobDispatcher(unsigned int workerCount);
    ~RenderJobDispatcher();

    RenderJobDispatcher(const RenderJobDispatcher&) = delete;
    RenderJobDispatcher& operator=(const RenderJobDispatcher&) = delete;

    // Reserve the output prefix before executing any job that can write to it.
    void submit(const std::string& label, const MaterialX::FilePath& outputPrefix, Work work);
    std::vector<RenderJobResult> finish();

  private:
    void close();
    void run();

    unsigned int _workerCount;
    std::vector<std::thread> _threads;
    std::deque<std::packaged_task<RenderJobResult()>> _queue;
    std::vector<std::future<RenderJobResult>> _results;
    std::set<std::string> _outputPaths;
    std::mutex _mutex;
    std::condition_variable _ready;
    std::condition_variable _space;
    bool _closed = false;
};

unsigned int availableCpuCount();
unsigned int oslThreadCount(int workers, int threadsPerWorker, unsigned int cpuCount);

} // namespace RenderUtil

#endif
