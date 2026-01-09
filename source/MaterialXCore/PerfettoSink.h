//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_PERFETTOSINK_H
#define MATERIALX_PERFETTOSINK_H

/// @file
/// Perfetto-based implementation of the Tracing::Sink interface.
///
/// Usage:
///   #include <MaterialXCore/PerfettoSink.h>
///   namespace trace = mx::Tracing;
///   auto sink = trace::PerfettoSink::create();
///   sink->initialize();
///   trace::Dispatcher::getInstance().setSink(sink);
///   // ... run application with tracing ...
///   trace::Dispatcher::getInstance().setSink(nullptr);
///   sink->shutdown("trace.perfetto-trace");
///   // Open the .perfetto-trace file at https://ui.perfetto.dev

#include <MaterialXCore/Tracing.h>

#ifdef MATERIALX_BUILD_TRACING

#include <memory>
#include <string>

MATERIALX_NAMESPACE_BEGIN

namespace Tracing
{

/// @class PerfettoSink
/// Perfetto-based implementation of Tracing::Sink.
///
/// This class provides a concrete implementation using Perfetto SDK's
/// in-process tracing backend. Trace data is written to a .perfetto-trace
/// file that can be visualized at https://ui.perfetto.dev
class MX_CORE_API PerfettoSink : public Sink
{
  public:
    /// Create a new Perfetto sink instance.
    static std::shared_ptr<PerfettoSink> create();

    ~PerfettoSink() override;

    /// Initialize Perfetto tracing. Must be called before any trace events.
    /// @param bufferSizeKb Size of the trace buffer in KB (default 32MB)
    void initialize(size_t bufferSizeKb = 32768);

    /// Stop tracing and write the trace to a file.
    /// @param outputPath Path to write the trace file (e.g., "trace.perfetto-trace")
    void shutdown(const std::string& outputPath);

    // Sink interface implementation
    void beginEvent(const char* category, const char* name) override;
    void endEvent(const char* category) override;
    void counter(const char* category, const char* name, double value) override;
    void setThreadName(const char* name) override;

  private:
    PerfettoSink();
    
    class Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace Tracing

MATERIALX_NAMESPACE_END

#endif // MATERIALX_BUILD_TRACING

#endif // MATERIALX_PERFETTOSINK_H

