# MaterialXGenOgsXml

This module generates OGS XML fragments that are compatible with Maya. These fragments can be registered using the Maya `MHWRender::MFragmentManager` API.

## Main entry point

The main class to use is OgsFragment. You use one of the two constructors to pass in the MaterialX::Element to wrap along with a proper shader generation environment and it will return the text of the fragment in `OgsFragment::getFragmentSource()` and its name in `OgsFragment::getFragmentName()`.

The OgsFragment will use the services of the OgsXmlFragment generator to extract the uniform and varying shading attributes of the shader code and expose them as OGS fragment inputs. The main entry point will gather all the values and pass them to the main MaterialX evaluation function.

## Maya light support

A fragment graph that integrates Maya lighting is provided for surface shaders via `OgsFragment::getLightRigSource()`. The name of this graph is stored in `OgsFragment::getLightRigName()`.

### Regular Maya lights

Up to 16 Maya lights are supported. Maya will add code to the OGS main function to compute lights and pass that result directly to a light integration fragment for immediate execution. Since the MaterialX light loop is found elsewhere, the light integration code found in `materialXLightDataBuilder.xml` will store the lights for later use. The light data is sent as a diffuse light component coupled with a light direction vector. We store the information as a MaterialX directional light since this is the simplest light fragment that will produce correct lighting results in the light loop.

### Environmental lights

Maya provides fragment entry points for environment lighting. The lighting graph will add a `diffuseI`, `specularI`, and `roughness` inputs that Maya will use whenever an environment light like the `Arnold Skydome Light` is found in the scene. This will result in code being added that computes an irradiance value that will be passed in the `diffuseI` parameter, and a glossy radiance value (computed for the current `roughness` value) in the `specularI` parameter. Since we care about the radiance for correct environment reflections, the default `roughness` value will be left at zero. This means we are missing a glossy environment value and can only blend between pure radiance and pure irradiance. This will cause incorrect roughness display where rougher surfaces will become more washed out in irradiance.

### Superior environmental lighting

Recent versions of Maya provide a new shader lighting API. Enabling the API in MaterialX requires calling `MaterialX::OgsXmlGenerator::setUseLightAPIV2(true);` before translating your MaterialX document.

The version 2 API relies on Maya injecting the following 5 functions into the shader code at compile time:

`int mayaGetNumLights()`\
Returns the number of active Maya lights in the scene at the moment the shader was last compiled.

`irradiance mayaGetLightIrradiance(int lightIndex, float3 Pw, float3 Nw, float3 Vw)`\
Returns a filled irradiance struct {`diffuseI`, `specularI`, `Ld`, `Ls`, `Lg`, `lightType`}\
`lightIndex` -> the index of the light to compute\
`Pw` -> world position of the surface point\
`Nw` -> world normal of the surface point (for rectArea lights)\
`Vw` -> world view direction (for rectArea lights)

`float3 mayaGetSpecularEnvironment(float3 Nw, float3 Vw, float phongExponent)`\
Returns a specular environment sample\
`Nw` -> world normal of the surface point\
`Vw` -> world view direction\
`phongExponent` -> \[0.0001, 2048\] tightness of the specular highlight

`float3 mayaGetIrradianceEnvironment(float3 Nw)`\
Returns an irradiance environment sample\
`Nw` -> world normal of the surface point

`float mayaRoughnessToPhongExp(float roughness)`\
Converts a normalized surface roughness value into a phongExponent compatible with mayaGetSpecularEnvironment()\
`roughness` -> \[0, 1\] Roughness of the surface

The OgsXml generator will fully use these function to provide improved illumination.
