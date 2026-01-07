//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_MXTRACEPERFETTO_H
#define MATERIALX_MXTRACEPERFETTO_H

/// @file
/// Perfetto-based implementation of the MxTraceBackend interface.
///
/// Usage:
///   #include <MaterialXCore/MxTracePerfetto.h>
///   auto backend = mx::MxPerfettoBackend::create();
///   backend->initialize();
///   mx::MxTraceCollector::getInstance().setBackend(backend);
///   // ... run application with tracing ...
///   mx::MxTraceCollector::getInstance().setBackend(nullptr);
///   backend->shutdown("trace.perfetto-trace");
///   // Open the .perfetto-trace file at https://ui.perfetto.dev

#include <MaterialXCore/MxTrace.h>

#ifdef MATERIALX_BUILD_TRACING

#include <memory>
#include <string>

MATERIALX_NAMESPACE_BEGIN

/// @class MxPerfettoBackend
/// Perfetto-based implementation of MxTraceBackend.
///
/// This class provides a concrete implementation using Perfetto SDK's
/// in-process tracing backend. Trace data is written to a .perfetto-trace
/// file that can be visualized at https://ui.perfetto.dev
class MX_CORE_API MxPerfettoBackend : public MxTraceBackend
{
  public:
    /// Create a new Perfetto backend instance.
    static std::shared_ptr<MxPerfettoBackend> create();

    ~MxPerfettoBackend() override;

    /// Initialize Perfetto tracing. Must be called before any trace events.
    /// @param bufferSizeKb Size of the trace buffer in KB (default 32MB)
    void initialize(size_t bufferSizeKb = 32768);

    /// Stop tracing and write the trace to a file.
    /// @param outputPath Path to write the trace file (e.g., "trace.perfetto-trace")
    void shutdown(const std::string& outputPath);

    // MxTraceBackend interface implementation
    void beginEvent(const char* category, const char* name) override;
    void endEvent(const char* category) override;
    void counter(const char* category, const char* name, double value) override;
    void setThreadName(const char* name) override;

  private:
    MxPerfettoBackend();
    
    class Impl;
    std::unique_ptr<Impl> _impl;
};

MATERIALX_NAMESPACE_END

#endif // MATERIALX_BUILD_TRACING

#endif // MATERIALX_MXTRACEPERFETTO_H

