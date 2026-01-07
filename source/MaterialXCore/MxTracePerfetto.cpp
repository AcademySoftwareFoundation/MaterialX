//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXCore/MxTracePerfetto.h>

#ifdef MATERIALX_BUILD_TRACING

#include <perfetto.h>

// Define Perfetto trace categories for MaterialX
// These must be in a .cpp file, not a header
PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category("mx.render")
        .SetDescription("MaterialX rendering operations"),
    perfetto::Category("mx.shadergen")
        .SetDescription("MaterialX shader generation"),
    perfetto::Category("mx.optimize")
        .SetDescription("MaterialX optimization passes")
);

// Required for Perfetto SDK - provides static storage for track events
PERFETTO_TRACK_EVENT_STATIC_STORAGE();

MATERIALX_NAMESPACE_BEGIN

class MxPerfettoBackend::Impl
{
  public:
    std::unique_ptr<perfetto::TracingSession> session;
};

MxPerfettoBackend::MxPerfettoBackend() : _impl(new Impl())
{
}

MxPerfettoBackend::~MxPerfettoBackend() = default;

std::shared_ptr<MxPerfettoBackend> MxPerfettoBackend::create()
{
    // Use shared_ptr with custom destructor to handle private constructor
    return std::shared_ptr<MxPerfettoBackend>(new MxPerfettoBackend());
}

void MxPerfettoBackend::initialize(size_t bufferSizeKb)
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

void MxPerfettoBackend::shutdown(const std::string& outputPath)
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

void MxPerfettoBackend::beginEvent(const char* category, const char* name)
{
    // Use dynamic category and name for flexibility
    // Note: For maximum performance, static categories/names should be used
    TRACE_EVENT_BEGIN(category, perfetto::DynamicString(name));
}

void MxPerfettoBackend::endEvent(const char* category)
{
    TRACE_EVENT_END(category);
}

void MxPerfettoBackend::counter(const char* category, const char* name, double value)
{
    TRACE_COUNTER(category, perfetto::DynamicString(name), value);
}

void MxPerfettoBackend::setThreadName(const char* name)
{
    // Set thread name for trace visualization
    auto track = perfetto::ThreadTrack::Current();
    auto desc = track.Serialize();
    desc.mutable_thread()->set_thread_name(name);
    perfetto::TrackEvent::SetTrackDescriptor(track, desc);
}

MATERIALX_NAMESPACE_END

#endif // MATERIALX_BUILD_TRACING

