#ifndef MATERIALXFORMAYA_AMBIENTOCCLUSION_H
#define MATERIALXFORMAYA_AMBIENTOCCLUSION_H

#include <NodeTranslator.h>

class AmbientOcclusion : public NodeTranslator
{
    DECLARE_NODE_TRANSLATOR(AmbientOcclusion)
public:
    virtual mx::NodePtr exportNode(const MObject& mayaNode, const string& outputType, mx::NodeGraphPtr parent, TranslatorContext& context);
};


#endif // MATERIALXFORMAYA_AMBIENTOCCLUSION_H
