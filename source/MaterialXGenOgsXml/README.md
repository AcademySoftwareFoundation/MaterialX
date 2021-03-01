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

If you have ever hooked an OpenGL or DirectX debugger and looked at the fragment code generated for environment lighting, you will have noticed that the environment computation depends on the following functions:

*   `SampleLatLongReflect` to fetch a radiance sample from the environment texture
*   `SampleLatLongVolumeReflect` to compute a glossy sample from the pre computed glossy 3D texture. The glossiness is controlled by a phong exponent
*   `adjustGain` to adjust the gain on the environment samples based provided uniform parameters
*   `mayaRoughnessToPhongExp` to compute a phong exponent from a (0..1) roughness value
*   `blendGlossyAndRadianceEnvironment` to compute the final radiance value based on previously computed radiance and glossy samples.

So it would be extremely nice if those functions could be used in `mx_environment_radiance()` to compute the light value. The big problem is that if there are no environment lights present in the scene, then none of these functions will be present and the compilation will fail. It would be extremely nice if Maya could somehow advertise the presence of these functions at the precompiler level. The deeply nested `#ifdef` code block in `GlslFragmeentGenerator::MX_ENVIRONMENT_MAYA` gives a hint of how this code could look for these functions in a future version of Maya, but it will not work at all with the current version of the software.
