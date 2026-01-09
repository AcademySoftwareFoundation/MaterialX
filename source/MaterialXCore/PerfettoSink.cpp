//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXCore/PerfettoSink.h>
#include <MaterialXCore/Tracing.h>

#ifdef MATERIALX_BUILD_TRACING

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

#include <fstream>
#include <cstring>

// Define Perfetto trace categories for MaterialX
// These must be in a .cpp file, not a header
PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category("mx.render")
        .SetDescription("MaterialX rendering operations"),
    perfetto::Category("mx.shadergen")
        .SetDescription("MaterialX shader generation"),
    perfetto::Category("mx.optimize")
        .SetDescription("MaterialX optimization passes"),
    perfetto::Category("mx.material")
        .SetDescription("MaterialX material identity markers")
);

// Required for Perfetto SDK - provides static storage for track events
PERFETTO_TRACK_EVENT_STATIC_STORAGE();

MATERIALX_NAMESPACE_BEGIN

namespace Tracing
{

class PerfettoSink::Impl
{
  public:
    std::unique_ptr<perfetto::TracingSession> session;
};

PerfettoSink::PerfettoSink() : _impl(new Impl())
{
}

PerfettoSink::~PerfettoSink() = default;

std::shared_ptr<PerfettoSink> PerfettoSink::create()
{
    // Use shared_ptr with custom destructor to handle private constructor
    return std::shared_ptr<PerfettoSink>(new PerfettoSink());
}

void PerfettoSink::initialize(size_t bufferSizeKb)
{
    // Initialize Perfetto with in-process backend
    perfetto::TracingInitArgs args;
    args.backends |= perfetto::kInProcessBackend;
    perfetto::Tracing::Initialize(args);

    // Register track event data source
    perfetto::TrackEvent::Register();

    // Configure tracing session
    perfetto::TraceConfig cfg;
    cfg.add_buffers()->set_size_kb(static_cast<uint32_t>(bufferSizeKb));

    auto* ds_cfg = cfg.add_data_sources()->mutable_config();
    ds_cfg->set_name("track_event");

    // Start tracing session
    _impl->session = perfetto::Tracing::NewTrace();
    _impl->session->Setup(cfg);
    _impl->session->StartBlocking();
}

void PerfettoSink::shutdown(const std::string& outputPath)
{
    if (!_impl->session)
        return;

    // Flush any pending trace data
    perfetto::TrackEvent::Flush();

    // Stop the tracing session
    _impl->session->StopBlocking();

    // Read trace data and write to file
    std::vector<char> traceData(_impl->session->ReadTraceBlocking());
    if (!traceData.empty())
    {
        std::ofstream output(outputPath, std::ios::binary);
        output.write(traceData.data(), static_cast<std::streamsize>(traceData.size()));
    }

    _impl->session.reset();
}

// Helper to check category match (pointer comparison first for constexpr, then strcmp)
static bool categoryMatches(const char* category, const char* target)
{
    return category == target || (category && std::strcmp(category, target) == 0);
}

void PerfettoSink::beginEvent(const char* category, const char* name)
{
    // Perfetto requires compile-time category names for TRACE_EVENT macros.
    // We dispatch based on the runtime category to the appropriate compile-time macro.
    // Pointer comparison catches constexpr usage, strcmp handles dynamic strings.
    if (categoryMatches(category, Category::ShaderGen))
    {
        TRACE_EVENT_BEGIN("mx.shadergen", nullptr, [&](perfetto::EventContext ctx) {
            ctx.event()->set_name(name);
        });
    }
    else if (categoryMatches(category, Category::Optimize))
    {
        TRACE_EVENT_BEGIN("mx.optimize", nullptr, [&](perfetto::EventContext ctx) {
            ctx.event()->set_name(name);
        });
    }
    else if (categoryMatches(category, Category::Material))
    {
        TRACE_EVENT_BEGIN("mx.material", nullptr, [&](perfetto::EventContext ctx) {
            ctx.event()->set_name(name);
        });
    }
    else // Default to mx.render
    {
        TRACE_EVENT_BEGIN("mx.render", nullptr, [&](perfetto::EventContext ctx) {
            ctx.event()->set_name(name);
        });
    }
}

void PerfettoSink::endEvent(const char* category)
{
    // Must match the category used in beginEvent
    if (categoryMatches(category, Category::ShaderGen))
    {
        TRACE_EVENT_END("mx.shadergen");
    }
    else if (categoryMatches(category, Category::Optimize))
    {
        TRACE_EVENT_END("mx.optimize");
    }
    else if (categoryMatches(category, Category::Material))
    {
        TRACE_EVENT_END("mx.material");
    }
    else
    {
        TRACE_EVENT_END("mx.render");
    }
}

void PerfettoSink::counter(const char* category, const char* name, double value)
{
    (void)category;
    // Create a counter track with the given name
    auto track = perfetto::CounterTrack(name);
    TRACE_COUNTER("mx.render", track, value);
}

void PerfettoSink::setThreadName(const char* name)
{
    // Set thread name for trace visualization
    auto track = perfetto::ThreadTrack::Current();
    auto desc = track.Serialize();
    desc.mutable_thread()->set_thread_name(name);
    perfetto::TrackEvent::SetTrackDescriptor(track, desc);
}

} // namespace Tracing

MATERIALX_NAMESPACE_END

#endif // MATERIALX_BUILD_TRACING

