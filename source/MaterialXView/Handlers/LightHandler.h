#ifndef MATERIALX_LIGHTHANDLER_H
#define MATERIALX_LIGHTHANDLER_H

#include <MaterialXCore/Definition.h>

#include <string>
#include <memory>

namespace MaterialX
{

class HwShaderGenerator;

/// Shared pointer to a LightSource
using LightSourcePtr = std::shared_ptr<class LightSource>;

/// @class @LightSource
/// Class holding data for a light source.
///
class LightSource
{
public:
    using Type = size_t;
    using ParameterMap = std::unordered_map<string, ValuePtr>;

    /// Return the light type
    Type getType() const
    {
        return _type;
    }

    /// Return the light parameters
    const ParameterMap& getParameters() const
    {
        return _parameters;
    }

    /// Set a light parameter value
    template<typename T>
    void setParameter(const string& name, const T& value)
    {
        ParameterMap::iterator it = _parameters.find(name);
        if (it != _parameters.end())
        {
            if (!it->second)
            {
                it->second = Value::createValue<T>(value);
            }
            else
            {
                TypedValue<T>* typedVal = dynamic_cast<TypedValue<T>*>(it->second.get());
                if (!typedVal)
                {
                    throw Exception("Incorrect type when setting light paramater");
                }
                typedVal->setData(value);
            }
        }
    }

protected:
    LightSource(Type type, const NodeDef& nodeDef);

    const Type _type;
    ParameterMap _parameters;

    friend class LightHandler;
};


/// Shared pointer to a LightHandler
using LightHandlerPtr = std::shared_ptr<class LightHandler>;

/// @class @LightHandler
/// Utility light handler for creating and providing 
/// light data for shader binding.
///
class LightHandler
{
public:
    using LightShaderMap = std::unordered_map<ConstNodeDefPtr, LightSource::Type>;

public:
    /// Static instance creator
    static LightHandlerPtr creator() { return std::make_shared<LightHandler>(); }

    /// Default constructor
    LightHandler();
    
    /// Default destructor
    virtual ~LightHandler();

    void bindLightShaders(HwShaderGenerator& shadergen) const;

    LightSourcePtr createLightSource(ConstNodeDefPtr nodeDef);

    const vector<LightSourcePtr>& getLightSources() const
    {
        return _lightSources;
    }

private:
    LightShaderMap _lightShaders;
    vector<LightSourcePtr> _lightSources;
};

} // namespace MaterialX

#endif
