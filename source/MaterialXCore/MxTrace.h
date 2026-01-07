//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_MXTRACE_H
#define MATERIALX_MXTRACE_H

/// @file
/// Tracing infrastructure for performance analysis.
/// 
/// This module provides an abstract tracing interface that can be backed by
/// different implementations (Perfetto, USD TraceCollector, etc.).
///
/// Design goals:
/// - API similar to USD's TraceCollector/TraceScope for familiarity
/// - Abstract backend allows USD to inject its own tracing when calling MaterialX
/// - Zero overhead when tracing is disabled (macros compile to nothing)

#include <MaterialXCore/Export.h>

#include <memory>
#include <string>

MATERIALX_NAMESPACE_BEGIN

// Trace category constants (similar to USD's TraceCategory)
#define MX_TRACE_CAT_RENDER    "mx.render"
#define MX_TRACE_CAT_SHADERGEN "mx.shadergen"
#define MX_TRACE_CAT_OPTIMIZE  "mx.optimize"

/// @class MxTraceBackend
/// Abstract tracing backend interface.
/// 
/// Implementations can delegate to Perfetto, USD TraceCollector, or custom systems.
/// This allows USD/Hydra to inject their own tracing when calling MaterialX code.
class MX_CORE_API MxTraceBackend
{
  public:
    virtual ~MxTraceBackend() = default;

    /// Begin a trace event with the given category and name.
    virtual void beginEvent(const char* category, const char* name) = 0;

    /// End the current trace event for the given category.
    virtual void endEvent(const char* category) = 0;

    /// Record a counter value (e.g., GPU time, memory usage).
    virtual void counter(const char* category, const char* name, double value) = 0;

    /// Set the current thread's name for trace visualization.
    virtual void setThreadName(const char* name) = 0;
};

/// @class MxTraceCollector
/// Global trace collector singleton (similar to USD's TraceCollector).
/// 
/// Usage:
///   MxTraceCollector::getInstance().setBackend(myBackend);
///   MxTraceCollector::getInstance().beginEvent("mx.render", "RenderFrame");
class MX_CORE_API MxTraceCollector
{
  public:
    /// Get the singleton instance.
    static MxTraceCollector& getInstance();

    /// Set the tracing backend. Pass nullptr to disable tracing.
    void setBackend(std::shared_ptr<MxTraceBackend> backend);

    /// Get the current backend (may be nullptr).
    MxTraceBackend* getBackend() const { return _backend.get(); }

    /// Check if tracing is currently enabled.
    bool isEnabled() const { return _backend != nullptr; }

    /// Begin a trace event.
    void beginEvent(const char* category, const char* name)
    {
        if (_backend)
            _backend->beginEvent(category, name);
    }

    /// End a trace event.
    void endEvent(const char* category)
    {
        if (_backend)
            _backend->endEvent(category);
    }

    /// Record a counter value.
    void counter(const char* category, const char* name, double value)
    {
        if (_backend)
            _backend->counter(category, name, value);
    }

  private:
    MxTraceCollector() = default;
    MxTraceCollector(const MxTraceCollector&) = delete;
    MxTraceCollector& operator=(const MxTraceCollector&) = delete;

    std::shared_ptr<MxTraceBackend> _backend;
};

/// @class MxTraceScope
/// RAII scope guard for trace events (similar to USD's TraceScope).
/// 
/// Usage:
///   {
///       MxTraceScope scope("mx.render", "RenderMaterial");
///       // ... code to trace ...
///   } // Event automatically ends here
class MX_CORE_API MxTraceScope
{
  public:
    MxTraceScope(const char* category, const char* name)
        : _category(category)
        , _enabled(MxTraceCollector::getInstance().isEnabled())
    {
        if (_enabled)
            MxTraceCollector::getInstance().beginEvent(category, name);
    }

    ~MxTraceScope()
    {
        if (_enabled)
            MxTraceCollector::getInstance().endEvent(_category);
    }

    // Non-copyable
    MxTraceScope(const MxTraceScope&) = delete;
    MxTraceScope& operator=(const MxTraceScope&) = delete;

  private:
    const char* _category;
    bool _enabled;
};

MATERIALX_NAMESPACE_END

// ============================================================================
// Tracing Macros
// ============================================================================
// When MATERIALX_BUILD_TRACING is defined, these macros generate trace events.
// Otherwise, they compile to nothing (zero overhead).

#ifdef MATERIALX_BUILD_TRACING

/// Create a scoped trace event. Event ends when scope exits.
#define MX_TRACE_SCOPE(category, name) \
    MaterialX::MxTraceScope _mxTraceScope##__LINE__(category, name)

/// Create a scoped trace event using the current function name.
#define MX_TRACE_FUNCTION(category) \
    MX_TRACE_SCOPE(category, __FUNCTION__)

/// Record a counter value.
#define MX_TRACE_COUNTER(category, name, value) \
    MaterialX::MxTraceCollector::getInstance().counter(category, name, value)

/// Begin a trace event (must be paired with MX_TRACE_END).
#define MX_TRACE_BEGIN(category, name) \
    MaterialX::MxTraceCollector::getInstance().beginEvent(category, name)

/// End a trace event.
#define MX_TRACE_END(category) \
    MaterialX::MxTraceCollector::getInstance().endEvent(category)

#else // MATERIALX_BUILD_TRACING not defined

#define MX_TRACE_SCOPE(category, name)
#define MX_TRACE_FUNCTION(category)
#define MX_TRACE_COUNTER(category, name, value)
#define MX_TRACE_BEGIN(category, name)
#define MX_TRACE_END(category)

#endif // MATERIALX_BUILD_TRACING

#endif // MATERIALX_MXTRACE_H

