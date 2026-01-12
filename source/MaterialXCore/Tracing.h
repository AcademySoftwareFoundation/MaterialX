//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_TRACING_H
#define MATERIALX_TRACING_H

/// @file
/// Tracing infrastructure for performance analysis.
/// 
/// This module provides an abstract tracing interface that can be backed by
/// different implementations (Perfetto, USD TraceCollector, etc.).
///
/// Design goals:
/// - API similar to USD's TraceCollector/TraceScope for familiarity
/// - Abstract sink allows USD to inject its own tracing when calling MaterialX
/// - Zero overhead when tracing is disabled (macros compile to nothing)
/// - Constexpr category strings for compile-time validation

#include <MaterialXCore/Export.h>

#include <cassert>
#include <memory>
#include <string>

MATERIALX_NAMESPACE_BEGIN

/// @namespace Tracing
/// Tracing infrastructure for performance analysis.
namespace Tracing
{

/// @namespace Category
/// Constexpr trace category identifiers.
/// 
/// These are compile-time constant strings that identify trace event categories.
/// Using constexpr ensures type safety and enables compile-time validation.
/// The USD TraceCollector backend can map these to its own categories.
namespace Category
{
    /// Rendering operations (GPU commands, frame capture, etc.)
    constexpr const char* Render = "mx.render";
    
    /// Shader generation (code generation, optimization passes)
    constexpr const char* ShaderGen = "mx.shadergen";
    
    /// Optimization passes (constant folding, dead code elimination)
    constexpr const char* Optimize = "mx.optimize";
    
    /// Material/shader identity markers (for filtering/grouping in traces)
    constexpr const char* Material = "mx.material";
}

// Usage: Add a namespace alias in your .cpp file for brevity:
//   namespace trace = MaterialX::Tracing;
//   MX_TRACE_SCOPE(trace::Category::Render, "MyEvent");

/// @class Sink
/// Abstract tracing sink interface.
/// 
/// Implementations can delegate to Perfetto, USD TraceCollector, or custom systems.
/// This allows USD/Hydra to inject their own tracing when calling MaterialX code.
class MX_CORE_API Sink
{
  public:
    virtual ~Sink() = default;

    /// Begin a trace event with the given category and name.
    virtual void beginEvent(const char* category, const char* name) = 0;

    /// End the current trace event for the given category.
    virtual void endEvent(const char* category) = 0;

    /// Record a counter value (e.g., GPU time, memory usage).
    virtual void counter(const char* category, const char* name, double value) = 0;

    /// Set the current thread's name for trace visualization.
    virtual void setThreadName(const char* name) = 0;
};

/// @class Dispatcher
/// Global trace dispatcher singleton.
/// 
/// Owns the active sink and dispatches trace events to it.
/// The Dispatcher takes ownership of the sink via unique_ptr.
///
/// Usage:
///   Dispatcher::getInstance().setSink(std::make_unique<PerfettoSink>(...));
///   // ... traced work ...
///   Dispatcher::getInstance().shutdownSink();
class MX_CORE_API Dispatcher
{
  public:
    /// Get the singleton instance.
    static Dispatcher& getInstance();

    /// Set the tracing sink. Takes ownership.
    /// Asserts if a sink is already set (call shutdownSink() first).
    void setSink(std::unique_ptr<Sink> sink);

    /// Shutdown and destroy the current sink.
    /// The sink's destructor handles writing output.
    void shutdownSink();

    /// Check if tracing is currently enabled.
    bool isEnabled() const { return _sink != nullptr; }

    /// Begin a trace event.
    void beginEvent(const char* category, const char* name)
    {
        if (_sink)
            _sink->beginEvent(category, name);
    }

    /// End a trace event.
    void endEvent(const char* category)
    {
        if (_sink)
            _sink->endEvent(category);
    }

    /// Record a counter value.
    void counter(const char* category, const char* name, double value)
    {
        if (_sink)
            _sink->counter(category, name, value);
    }

  private:
    Dispatcher() = default;
    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;

    std::unique_ptr<Sink> _sink;
};

/// @class Scope
/// RAII scope guard for trace events (similar to USD's TraceScope).
/// 
/// Usage:
///   {
///       Tracing::Scope scope("mx.render", "RenderMaterial");
///       // ... code to trace ...
///   } // Event automatically ends here
class MX_CORE_API Scope
{
  public:
    Scope(const char* category, const char* name)
        : _category(category)
    {
        Dispatcher::getInstance().beginEvent(category, name);
    }

    ~Scope()
    {
        Dispatcher::getInstance().endEvent(_category);
    }

    // Non-copyable
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;

  private:
    const char* const _category;
};

} // namespace Tracing

MATERIALX_NAMESPACE_END

// ============================================================================
// Tracing Macros
// ============================================================================
// When MATERIALX_BUILD_TRACING is defined, these macros generate trace events.
// Otherwise, they compile to nothing (zero overhead).

// Helper macros for token pasting with __LINE__ expansion
#define MX_TRACE_CONCAT_IMPL(a, b) a##b
#define MX_TRACE_CONCAT(a, b) MX_TRACE_CONCAT_IMPL(a, b)

#ifdef MATERIALX_BUILD_TRACING

/// Create a scoped trace event. Event ends when scope exits.
#define MX_TRACE_SCOPE(category, name) \
    MaterialX::Tracing::Scope MX_TRACE_CONCAT(_mxTraceScope_, __LINE__)(category, name)

/// Create a scoped trace event using the current function name.
#define MX_TRACE_FUNCTION(category) \
    MaterialX::Tracing::Scope MX_TRACE_CONCAT(_mxTraceFn_, __LINE__)(category, __FUNCTION__)

/// Record a counter value.
#define MX_TRACE_COUNTER(category, name, value) \
    MaterialX::Tracing::Dispatcher::getInstance().counter(category, name, value)

/// Begin a trace event (must be paired with MX_TRACE_END).
#define MX_TRACE_BEGIN(category, name) \
    MaterialX::Tracing::Dispatcher::getInstance().beginEvent(category, name)

/// End a trace event.
#define MX_TRACE_END(category) \
    MaterialX::Tracing::Dispatcher::getInstance().endEvent(category)

#else // MATERIALX_BUILD_TRACING not defined

#define MX_TRACE_SCOPE(category, name)
#define MX_TRACE_FUNCTION(category)
#define MX_TRACE_COUNTER(category, name, value)
#define MX_TRACE_BEGIN(category, name)
#define MX_TRACE_END(category)

#endif // MATERIALX_BUILD_TRACING

#endif // MATERIALX_TRACING_H
