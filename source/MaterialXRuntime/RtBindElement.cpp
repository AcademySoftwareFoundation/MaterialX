//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtBindElement.h>
#include <MaterialXRuntime/RtApi.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

DEFINE_TYPED_SCHEMA(RtBindElement, "bindelement");

RtPrim RtBindElement::createPrim(const RtToken&, const RtToken&, RtPrim)
{
    throw ExceptionRuntimeError("RtBindElement is an abstract base class and cannot create prims");
}

const RtPrimSpec& RtBindElement::getPrimSpec() const
{
    static const PvtPrimSpec s_primSpec;
    return s_primSpec;
}

}
