//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTrace/PerfettoSink.h>

#ifdef MATERIALX_BUILD_PERFETTO_TRACING

#include <cstdint>
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

// ---------------------------------------------------------------------------
// Per-category Perfetto operations.
//
// Perfetto trace macros require compile-time category string literals, so
// we stamp out a thin ops struct per category and dispatch through a generic
// lambda.  Adding a new Category means adding one DEFINE line and one switch
// case in withCategory() below.
// ---------------------------------------------------------------------------
namespace
{

#define MX_DEFINE_PERFETTO_OPS(Name, perfettoCategory)                           \
struct Name                                                                      \
{                                                                                \
    static void beginEvent(const char* name)                                     \
    {                                                                            \
        TRACE_EVENT_BEGIN(perfettoCategory, nullptr,                              \
            [&](perfetto::EventContext ctx) { ctx.event()->set_name(name); });    \
    }                                                                            \
    static void endEvent()                                                       \
    {                                                                            \
        TRACE_EVENT_END(perfettoCategory);                                       \
    }                                                                            \
    static void counter(perfetto::CounterTrack track, double value)              \
    {                                                                            \
        TRACE_COUNTER(perfettoCategory, track, value);                           \
    }                                                                            \
    static void asyncBegin(const perfetto::Track& track, uint64_t startNs,       \
                           const char* name)                                     \
    {                                                                            \
        TRACE_EVENT_BEGIN(perfettoCategory, nullptr, track, startNs,             \
            [&](perfetto::EventContext ctx) { ctx.event()->set_name(name); });   \
    }                                                                            \
    static void asyncEnd(const perfetto::Track& track, uint64_t endNs)           \
    {                                                                            \
        TRACE_EVENT_END(perfettoCategory, track, endNs);                         \
    }                                                                            \
};

MX_DEFINE_PERFETTO_OPS(RenderOps,    "mx.render")
MX_DEFINE_PERFETTO_OPS(ShaderGenOps, "mx.shadergen")
MX_DEFINE_PERFETTO_OPS(OptimizeOps,  "mx.optimize")
MX_DEFINE_PERFETTO_OPS(MaterialOps,  "mx.material")

#undef MX_DEFINE_PERFETTO_OPS

/// Dispatch a generic callable by category.  The callable receives a
/// category-ops tag whose static methods wrap the Perfetto trace macros.
template<typename Fn>
void withCategory(Category category, Fn&& fn)
{
    switch (category)
    {
        case Category::Render:    fn(RenderOps{}); break;
        case Category::ShaderGen: fn(ShaderGenOps{}); break;
        case Category::Optimize:  fn(OptimizeOps{}); break;
        case Category::Material:  fn(MaterialOps{}); break;
        default: break;
    }
}

} // anonymous namespace

PerfettoSink::PerfettoSink(std::string outputPath,
                           const AsyncTrackMap& asyncTracks,
                           size_t bufferSizeKb)
    : _outputPath(std::move(outputPath))
{
    // One-time global Perfetto initialization
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        perfetto::TracingInitArgs args;
        args.backends |= perfetto::kInProcessBackend;
        perfetto::Tracing::Initialize(args);
        perfetto::TrackEvent::Register();
    });

    // Register async track descriptors from the caller-provided map.
    // Explicitly parent to the process track to avoid hierarchy loops.
    for (const auto& [id, name] : asyncTracks)
    {
        perfetto::Track perfTrack(id, perfetto::ProcessTrack::Current());
        auto desc = perfTrack.Serialize();
        desc.set_name(name);
        perfetto::TrackEvent::SetTrackDescriptor(perfTrack, desc);
        _asyncTracks.emplace(id, perfTrack);
    }

    // Create and start a tracing session
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
    withCategory(category, [name](auto ops) {
        decltype(ops)::beginEvent(name);
    });
}

void PerfettoSink::endEvent(Category category)
{
    withCategory(category, [](auto ops) {
        decltype(ops)::endEvent();
    });
}

void PerfettoSink::counter(Category category, const char* name, double value)
{
    withCategory(category, [name, value](auto ops) {
        decltype(ops)::counter(perfetto::CounterTrack(name), value);
    });
}

void PerfettoSink::asyncEvent(AsyncTrackId track, Category category,
                              const char* eventName, uint64_t startNs, uint64_t durationNs)
{
    auto it = _asyncTracks.find(track);
    if (it == _asyncTracks.end())
        return;

    const auto& perfTrack = it->second;
    uint64_t endNs = startNs + durationNs;

    withCategory(category, [&](auto ops) {
        decltype(ops)::asyncBegin(perfTrack, startNs, eventName);
        decltype(ops)::asyncEnd(perfTrack, endNs);
    });
}

uint64_t PerfettoSink::getTraceTimeNs()
{
    return perfetto::TrackEvent::GetTraceTimeNs();
}

void PerfettoSink::setThreadName(const char* name)
{
    // Set thread name for trace visualization
    auto track = perfetto::ThreadTrack::Current();
    auto desc = track.Serialize();
    desc.mutable_thread()->set_thread_name(name);
    perfetto::TrackEvent::SetTrackDescriptor(track, desc);
}

// Factory function - the exported entry point
std::unique_ptr<Sink> createPerfettoSink(const std::string& outputPath,
                                          const AsyncTrackMap& asyncTracks,
                                          size_t bufferSizeKb)
{
    return std::make_unique<PerfettoSink>(outputPath, asyncTracks, bufferSizeKb);
}

} // namespace Tracing

MATERIALX_NAMESPACE_END

#endif // MATERIALX_BUILD_PERFETTO_TRACING
