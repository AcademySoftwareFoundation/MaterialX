//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXCore/PerfettoSink.h>

#ifdef MATERIALX_BUILD_TRACING

#include <fstream>
#include <mutex>

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

// One-time global Perfetto initialization flag
static std::once_flag g_perfettoInitFlag;

PerfettoSink::PerfettoSink(std::string outputPath, size_t bufferSizeKb)
    : _outputPath(std::move(outputPath))
{
    // One-time global Perfetto initialization (safe to call from multiple instances)
    std::call_once(g_perfettoInitFlag, []() {
        perfetto::TracingInitArgs args;
        args.backends |= perfetto::kInProcessBackend;
        perfetto::Tracing::Initialize(args);
        perfetto::TrackEvent::Register();
    });

    // Create and start a tracing session for this sink
    perfetto::TraceConfig cfg;
    cfg.add_buffers()->set_size_kb(static_cast<uint32_t>(bufferSizeKb));

    auto* ds_cfg = cfg.add_data_sources()->mutable_config();
    ds_cfg->set_name("track_event");

    _session = perfetto::Tracing::NewTrace();
    _session->Setup(cfg);
    _session->StartBlocking();
}

PerfettoSink::~PerfettoSink()
{
    if (!_session)
        return;

    // Flush any pending trace data
    perfetto::TrackEvent::Flush();

    // Stop the tracing session
    _session->StopBlocking();

    // Read trace data and write to file
    std::vector<char> traceData(_session->ReadTraceBlocking());
    if (!traceData.empty())
    {
        std::ofstream output(_outputPath, std::ios::binary);
        output.write(traceData.data(), static_cast<std::streamsize>(traceData.size()));
    }
}

void PerfettoSink::beginEvent(Category category, const char* name)
{
    // Perfetto requires compile-time category names for TRACE_EVENT macros.
    // Switch on the enum lets the compiler optimize to a jump table.
    switch (category)
    {
        case Category::Render:
            TRACE_EVENT_BEGIN("mx.render", nullptr, [&](perfetto::EventContext ctx) {
                ctx.event()->set_name(name);
            });
            break;
        case Category::ShaderGen:
            TRACE_EVENT_BEGIN("mx.shadergen", nullptr, [&](perfetto::EventContext ctx) {
                ctx.event()->set_name(name);
            });
            break;
        case Category::Optimize:
            TRACE_EVENT_BEGIN("mx.optimize", nullptr, [&](perfetto::EventContext ctx) {
                ctx.event()->set_name(name);
            });
            break;
        case Category::Material:
            TRACE_EVENT_BEGIN("mx.material", nullptr, [&](perfetto::EventContext ctx) {
                ctx.event()->set_name(name);
            });
            break;
        default:
            // Fallback for any future categories
            TRACE_EVENT_BEGIN("mx.render", nullptr, [&](perfetto::EventContext ctx) {
                ctx.event()->set_name(name);
            });
            break;
    }
}

void PerfettoSink::endEvent(Category category)
{
    switch (category)
    {
        case Category::Render:
            TRACE_EVENT_END("mx.render");
            break;
        case Category::ShaderGen:
            TRACE_EVENT_END("mx.shadergen");
            break;
        case Category::Optimize:
            TRACE_EVENT_END("mx.optimize");
            break;
        case Category::Material:
            TRACE_EVENT_END("mx.material");
            break;
        default:
            TRACE_EVENT_END("mx.render");
            break;
    }
}

void PerfettoSink::counter(Category category, const char* name, double value)
{
    auto track = perfetto::CounterTrack(name);
    switch (category)
    {
        case Category::Render:
            TRACE_COUNTER("mx.render", track, value);
            break;
        case Category::ShaderGen:
            TRACE_COUNTER("mx.shadergen", track, value);
            break;
        case Category::Optimize:
            TRACE_COUNTER("mx.optimize", track, value);
            break;
        case Category::Material:
            TRACE_COUNTER("mx.material", track, value);
            break;
        default:
            TRACE_COUNTER("mx.render", track, value);
            break;
    }
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
