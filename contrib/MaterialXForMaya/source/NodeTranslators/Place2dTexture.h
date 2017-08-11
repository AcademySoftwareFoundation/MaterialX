#ifndef MATERIALXFORMAYA_PLACE2DTEXTURE_H
#define MATERIALXFORMAYA_PLACE2DTEXTURE_H

#include <NodeTranslator.h>

class Place2dTexture : public NodeTranslator
{
    DECLARE_NODE_TRANSLATOR(Place2dTexture)
public:
    virtual mx::NodePtr exportNode(const MObject& mayaNode, const string& outputType, mx::NodeGraphPtr parent, TranslatorContext& context);
};


#endif // MATERIALXFORMAYA_PLACE2DTEXTURE_H
