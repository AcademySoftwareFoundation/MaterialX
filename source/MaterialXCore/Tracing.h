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
/// - Enum-based categories for type safety and efficient dispatch

#include <MaterialXCore/Export.h>

#include <cassert>
#include <cstddef>
#include <memory>
#include <string>

MATERIALX_NAMESPACE_BEGIN

/// @namespace Tracing
/// Tracing infrastructure for performance analysis.
namespace Tracing
{

/// @enum Category
/// Trace event categories for filtering and organization.
/// 
/// These are used to categorize trace events for filtering in the UI
/// and to enable/disable specific categories at runtime.
enum class Category
{
    /// Rendering operations (GPU commands, frame capture, etc.)
    Render = 0,
    
    /// Shader generation (code generation, optimization passes)
    ShaderGen,
    
    /// Optimization passes (constant folding, dead code elimination)
    Optimize,
    
    /// Material/shader identity markers (for filtering/grouping in traces)
    Material,
    
    /// Number of categories (must be last)
    Count
};

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
    virtual void beginEvent(Category category, const char* name) = 0;

    /// End the current trace event for the given category.
    virtual void endEvent(Category category) = 0;

    /// Record a counter value (e.g., GPU time, memory usage).
    virtual void counter(Category category, const char* name, double value) = 0;

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

    /// Scope guard that calls shutdownSink() on destruction.
    /// Ensures tracing is properly shut down on any exit path (return, exception, etc.)
    ///
    /// Usage:
    ///   Dispatcher::getInstance().setSink(std::make_unique<PerfettoSink>(...));
    ///   Dispatcher::ShutdownGuard guard;
    ///   // ... traced work ...
    ///   // guard destructor calls shutdownSink()
    struct ShutdownGuard
    {
        ~ShutdownGuard() { Dispatcher::getInstance().shutdownSink(); }
        ShutdownGuard() = default;
        ShutdownGuard(const ShutdownGuard&) = delete;
        ShutdownGuard& operator=(const ShutdownGuard&) = delete;
    };

    /// Check if tracing is currently enabled.
    bool isEnabled() const { return _sink != nullptr; }

    /// Begin a trace event.
    void beginEvent(Category category, const char* name)
    {
        if (_sink)
            _sink->beginEvent(category, name);
    }

    /// End a trace event.
    void endEvent(Category category)
    {
        if (_sink)
            _sink->endEvent(category);
    }

    /// Record a counter value.
    void counter(Category category, const char* name, double value)
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
/// Template parameter Cat is the category enum value, known at compile time.
/// This avoids storing the category on the stack.
///
/// Usage:
///   {
///       Tracing::Scope<Category::Render> scope("RenderMaterial");
///       // ... code to trace ...
///   } // Event automatically ends here
template<Category Cat>
class Scope
{
  public:
    explicit Scope(const char* name)
    {
        Dispatcher::getInstance().beginEvent(Cat, name);
    }

    ~Scope()
    {
        Dispatcher::getInstance().endEvent(Cat);
    }

    // Non-copyable
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
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
/// Category must be a Tracing::Category enum value.
#define MX_TRACE_SCOPE(category, name) \
    MaterialX::Tracing::Scope<category> MX_TRACE_CONCAT(_mxTraceScope_, __LINE__)(name)

/// Create a scoped trace event using the current function name.
#define MX_TRACE_FUNCTION(category) \
    MaterialX::Tracing::Scope<category> MX_TRACE_CONCAT(_mxTraceFn_, __LINE__)(__FUNCTION__)

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
