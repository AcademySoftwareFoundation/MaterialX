//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/UnitSystem.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

namespace MaterialX
{

// Helper class to create the constant block for mx_distance_unit
class DistanceUnitNode : public SourceCodeNode
{
  public:
    explicit DistanceUnitNode(DefaultUnitConverterPtr distanceUnitConverter) :
         _distanceUnitConverter(distanceUnitConverter) {}

    static ShaderNodeImplPtr create(DefaultUnitConverterPtr distanceUnitConverter);

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  protected:
    DefaultUnitConverterPtr _distanceUnitConverter;
};

ShaderNodeImplPtr DistanceUnitNode::create(DefaultUnitConverterPtr distanceUnitConverter)
{
    return std::make_shared<DistanceUnitNode>(distanceUnitConverter);
}

void DistanceUnitNode::emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    // Emit the helper funtion unit_ratio that embeds a look up table for unit scale
    vector<float> distanceUnitScales;
    distanceUnitScales.reserve(_distanceUnitConverter->getUnitScale().size());
    for (auto scaleValue : _distanceUnitConverter->getUnitScale()) {
        distanceUnitScales.push_back(scaleValue.second);
    }
    // see stdlib/gen*/mx_DISTANCE_UNIT. This helper function is called by these shaders.
    const string VAR_DISTANCE_UNIT_SCALE = "u_distance_unit_scales";
    VariableBlock distanceUnitLUT("unitLUT", EMPTY_STRING);
    distanceUnitLUT.add(Type::FLOATARRAY, VAR_DISTANCE_UNIT_SCALE, Value::createValue<vector<float>>(distanceUnitScales));

    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    shadergen.emitString("float unit_ratio(int unit_from, int unit_to)", stage);
    shadergen.emitLineBreak(stage);
    shadergen.emitScopeBegin(stage);

    shadergen.emitLineBreak(stage);
    shadergen.emitVariableDeclarations(distanceUnitLUT, shadergen.getSyntax().getConstantQualifier(), ";", context, stage, true);

    shadergen.emitLineBreak(stage);
    shadergen.emitString("return ("+ VAR_DISTANCE_UNIT_SCALE + "[unit_from] / " + VAR_DISTANCE_UNIT_SCALE +"[unit_to]);", stage);
    shadergen.emitLineBreak(stage);
    shadergen.emitScopeEnd(stage);
    shadergen.emitLineBreak(stage);
    END_SHADER_STAGE(shader, Stage::PIXEL)

    // Emit registered implementation
    SourceCodeNode::emitFunctionDefinition(node, context, stage);
}

//
// Unit transform methods
//

UnitTransform::UnitTransform(const string& ss, const string& ts, const TypeDesc* t, const string& unittype) :
    sourceUnit(ss),
    targetUnit(ts),
    type(t),
    unitType(unittype)
{
    if (type != Type::FLOAT && type != Type::VECTOR2 && type != Type::VECTOR3 && type != Type::VECTOR4)
    {
        throw ExceptionShaderGenError("Unit space transform can only be a float or vectors");
    }
}

const string UnitSystem::UNITSYTEM_NAME = "default_unit_system";
const string UnitSystem::DISTANCE_UNIT_TARGET_NAME = "u_distanceUnitTarget";

UnitSystem::UnitSystem(const string& language)
{
    _language = createValidName(language);
}

void UnitSystem::loadLibrary(DocumentPtr document)
{
    _document = document;
}

void UnitSystem::setUnitConverterRegistry(UnitConverterRegistryPtr registry)
{
    _unitRegistry = registry;
}

UnitConverterRegistryPtr UnitSystem::getUnitConverterRegistry() const
{
    return _unitRegistry;
}

UnitSystemPtr UnitSystem::create(const string& language)
{
    UnitSystemPtr result(new UnitSystem(language));
    return result;
}

string UnitSystem::getImplementationName(const UnitTransform& transform, const string& unitname) const
{
    return "IM_" + unitname + "_unit_" + transform.type->getName() + "_" + _language;
}

bool UnitSystem::supportsTransform(const UnitTransform& transform) const
{
    const string implName = getImplementationName(transform, DefaultUnitConverter::DISTANCE_UNIT);
    ImplementationPtr impl = _document->getImplementation(implName);
    return impl != nullptr;
}

ShaderNodePtr UnitSystem::createNode(ShaderGraph* parent, const UnitTransform& transform, const string& name,
    GenContext& context) const
{
    const string implName = getImplementationName(transform, DefaultUnitConverter::DISTANCE_UNIT);
    ImplementationPtr impl = _document->getImplementation(implName);
    if (!impl)
    {
        throw ExceptionShaderGenError("No implementation found for transform: ('" + transform.sourceUnit + "', '" + transform.targetUnit + "').");
    }

    // Distance unit conversion
    UnitTypeDefPtr distanceTypeDef = _document->getUnitTypeDef(DefaultUnitConverter::DISTANCE_UNIT);
    if (!_unitRegistry && !_unitRegistry->getUnitConverter(distanceTypeDef))
    {
        throw ExceptionTypeError("Unit registry unavaliable or undefined unit converter for: " + DefaultUnitConverter::DISTANCE_UNIT);
    }
    DefaultUnitConverterPtr distanceConverter = std::dynamic_pointer_cast<DefaultUnitConverter>(_unitRegistry->getUnitConverter(distanceTypeDef));

    // Check if it's created and cached already,
    // otherwise create and cache it.
    ShaderNodeImplPtr nodeImpl = context.findNodeImplementation(implName);
    if (!nodeImpl)
    {
        nodeImpl = DistanceUnitNode::create(distanceConverter);
        nodeImpl->initialize(*impl, context);
        context.addNodeImplementation(implName, nodeImpl);
    }

    // Create the node.
    ShaderNodePtr shaderNode = ShaderNode::create(parent, name, nodeImpl, ShaderNode::Classification::TEXTURE);

    // Create ports on the node.
    ShaderInput* input = shaderNode->addInput("in", transform.type);
    if (transform.type == Type::FLOAT)
    {
        input->setValue(Value::createValue(1.0));
    }
    else if (transform.type == Type::VECTOR2)
    {
        input->setValue(Value::createValue(Vector2(1.0f, 1.0)));
    }
    else if (transform.type == Type::VECTOR3)
    {
        input->setValue(Value::createValue(Vector3(1.0f, 1.0, 1.0)));
    }
    else if (transform.type == Type::VECTOR4)
    {
        input->setValue(Value::createValue(Vector4(1.0f, 1.0, 1.0, 1.0)));
    }
    else
    {
        throw ExceptionShaderGenError("Invalid type specified to unitTransform: '" + transform.type->getName() + "'");
    }

    // Add the conversion code
    {
        int value = distanceConverter->getUnitAsInteger(transform.sourceUnit);
        if (value < 0)
        {
            throw ExceptionTypeError("Unrecognized source unit: " + transform.sourceUnit);
        }

        ShaderInput* convertFrom = shaderNode->addInput("unit_from", Type::INTEGER);
        convertFrom->setValue(Value::createValue(value));
    }

    {
        int value = distanceConverter->getUnitAsInteger(transform.targetUnit);
        if (value < 0)
        {
            throw ExceptionTypeError("Unrecognized target unit: " + transform.targetUnit);
        }

        ShaderInput* convertTo = shaderNode->addInput("unit_to", Type::INTEGER);

        // Create a graph input to connect to the "unit_to" if it does not already exist.
        ShaderGraphInputSocket* globalInput = parent->getInputSocket(DISTANCE_UNIT_TARGET_NAME);
        if (!globalInput)
        {
            globalInput = parent->addInputSocket(DISTANCE_UNIT_TARGET_NAME, Type::INTEGER);
        }
        globalInput->setValue(Value::createValue(value));
        convertTo->makeConnection(globalInput);
    }

    shaderNode->addOutput("out", transform.type);

    return shaderNode;
}

} // namespace MaterialX
