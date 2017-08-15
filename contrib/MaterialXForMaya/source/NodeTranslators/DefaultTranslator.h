// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#ifndef MATERIALXFORMAYA_DEFAULTTRANSLATOR_H
#define MATERIALXFORMAYA_DEFAULTTRANSLATOR_H

#include <NodeTranslator.h>

namespace MaterialXForMaya
{ 

/// @class DefaultTranslator
/// Default translator 
class DefaultTranslator : public NodeTranslator
{
    DECLARE_NODE_TRANSLATOR(DefaultTranslator)
};

} // namespace MaterialXForMaya

#endif // MATERIALXFORMAYA_DEFAULTTRANSLATOR_H
