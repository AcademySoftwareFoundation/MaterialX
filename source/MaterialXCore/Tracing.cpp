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

void Dispatcher::setSink(std::shared_ptr<Sink> sink)
{
    _sink = sink;
}

} // namespace Tracing

MATERIALX_NAMESPACE_END

