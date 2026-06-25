//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_PERFETTOSINK_H
#define MATERIALX_PERFETTOSINK_H

/// @file
/// Perfetto-based implementation of the Tracing::Sink interface.
///
/// This header is internal to MaterialXTrace. Users should NOT include it
/// directly. Instead, use the createPerfettoSink() factory in Tracing.h:
///
///   #include <MaterialXTrace/Tracing.h>
///   auto sink = mx::Tracing::createPerfettoSink("trace.perfetto-trace");

#include <MaterialXTrace/Tracing.h>

#ifdef MATERIALX_BUILD_PERFETTO_TRACING

// Suppress verbose warnings from Perfetto SDK templates
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127) // conditional expression is constant
#pragma warning(disable : 4146) // unary minus on unsigned type
#pragma warning(disable : 4369) // enumerator value cannot be represented
#endif

#include <perfetto.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <memory>
#include <string>

MATERIALX_NAMESPACE_BEGIN

namespace Tracing
{

/// @class PerfettoSink
/// Perfetto-based implementation of Tracing::Sink.
///
/// Uses Perfetto SDK's in-process tracing backend. The constructor starts
/// a tracing session, and the destructor stops it and writes trace data
/// to a .perfetto-trace file viewable at https://ui.perfetto.dev
///
/// @note Do not use this class directly. Use createPerfettoSink() factory.
class PerfettoSink : public Sink
{
  public:
    /// Construct and start a Perfetto tracing session.
    /// @param outputPath Path to write the trace file when destroyed
    /// @param bufferSizeKb Size of the trace buffer in KB (default 32MB)
    explicit PerfettoSink(std::string outputPath, size_t bufferSizeKb = 32768);
    
    /// Stop tracing and write the trace to the output path.
    ~PerfettoSink() override;

    // Non-copyable, non-movable
    PerfettoSink(const PerfettoSink&) = delete;
    PerfettoSink& operator=(const PerfettoSink&) = delete;
    PerfettoSink(PerfettoSink&&) = delete;
    PerfettoSink& operator=(PerfettoSink&&) = delete;

    // Sink interface implementation
    void beginEvent(Category category, const char* name) override;
    void endEvent(Category category) override;
    void counter(Category category, const char* name, double value) override;
    void asyncEvent(AsyncTrack track, Category category,
                   const char* eventName, uint64_t startNs, uint64_t durationNs) override;
    void setThreadName(const char* name) override;

  private:
    const std::string _outputPath;
    std::unique_ptr<perfetto::TracingSession> _session;
};

} // namespace Tracing

MATERIALX_NAMESPACE_END

#endif // MATERIALX_BUILD_PERFETTO_TRACING

#endif // MATERIALX_PERFETTOSINK_H
