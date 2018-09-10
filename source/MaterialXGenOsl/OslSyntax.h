#ifndef MATERIALX_OSLSYNTAX_H
#define MATERIALX_OSLSYNTAX_H

#include <MaterialXGenShader/Syntax.h>

namespace MaterialX
{

/// Syntax class for OSL (Open Shading Language)
class OslSyntax : public Syntax
{
public:
    OslSyntax();

    static SyntaxPtr create() { return std::make_shared<OslSyntax>(); }

    const string& getOutputQualifier() const override;

    static const string OUTPUT_QUALIFIER;
    static const vector<string> VECTOR_MEMBERS;
    static const vector<string> VECTOR2_MEMBERS;
    static const vector<string> VECTOR4_MEMBERS;
    static const vector<string> COLOR2_MEMBERS;
    static const vector<string> COLOR4_MEMBERS;
};

} // namespace MaterialX

#endif
