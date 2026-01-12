//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXCore/Tracing.h>

MATERIALX_NAMESPACE_BEGIN

namespace Tracing
{

Dispatcher& Dispatcher::getInstance()
{
    static Dispatcher instance;
    return instance;
}

void Dispatcher::setSink(std::unique_ptr<Sink> sink)
{
    // Assert if a sink is already set - caller should shutdownSink() first.
    // This catches programming errors; if triggered, the old sink will still
    // write its output when destroyed by this assignment.
    assert(!_sink && "Sink already set - call shutdownSink() first");
    _sink = std::move(sink);
}

void Dispatcher::shutdownSink()
{
    _sink.reset();  // Destructor handles writing output
}

} // namespace Tracing

MATERIALX_NAMESPACE_END
