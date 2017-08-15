// Copyright 2017 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//
#ifndef MATERIALXFORMAYA_AMBIENTOCCLUSION_H
#define MATERIALXFORMAYA_AMBIENTOCCLUSION_H

#include <NodeTranslator.h>

namespace MaterialXForMaya
{
/// @class AmbientOcclusion
/// Custom translator for ambient occlusion node
class AmbientOcclusion : public NodeTranslator
{
    DECLARE_NODE_TRANSLATOR(AmbientOcclusion)
public:
    /// Custom node exporter
    virtual mx::NodePtr exportNode(const MObject& mayaNode, const string& outputType, mx::NodeGraphPtr parent, TranslatorContext& context);
};

} // namespace MaterialXForMaya

#endif // MATERIALXFORMAYA_AMBIENTOCCLUSION_H
