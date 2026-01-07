//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXCore/MxTrace.h>

MATERIALX_NAMESPACE_BEGIN

MxTraceCollector& MxTraceCollector::getInstance()
{
    static MxTraceCollector instance;
    return instance;
}

void MxTraceCollector::setBackend(std::shared_ptr<MxTraceBackend> backend)
{
    _backend = backend;
}

MATERIALX_NAMESPACE_END

