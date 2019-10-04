- *Nodedef*: ND_standard_surface_surfaceshader
- *Type*: surfaceshader
- *Doc*: Autodesk standard surface shader

| Name | Type | Default Value | UI min | UI max | UI group | Description | UI Advanced | Connectable |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| base | float | 0.800000011921 | 0.0 | 1.0 | Base | Multiplier on the intensity of the diffuse reflection. |  | true |
| base_color | color3 | 1, 1, 1 | 0,0,0 | 1,1,1 | Base | Color of the diffuse reflection. |  | true |
| diffuse_roughness | float | 0.0 | 0.0 | 1.0 | Base | Roughness of the diffuse reflection. Higher values cause the surface to appear flatter and darker. | true | true |
| metalness | float | 0.0 | 0.0 | 1.0 | Base | Specifies how metallic the material appears. At its maximum, the surface behaves like a metal, using fully specular reflection and complex fresnel. |  | true |
| specular | float | 1.0 | 0.0 | 1.0 | Specular | Multiplier on the intensity of the specular reflection. |  | true |
| specular_color | color3 | 1, 1, 1 | 0,0,0 | 1,1,1 | Specular | Color tint on the specular reflection. |  | true |
| specular_roughness | float | 0.20000000298 | 0.0 | 1.0 | Specular | The roughness of the specular reflection. Lower numbers produce sharper reflections, higher numbers produce blurrier reflections. |  | true |
| specular_IOR | float | 1.5 | 0.0 | 3.0 | Specular | Index of refraction for specular reflection. |  | true |
| specular_anisotropy | float | 0.0 | 0.0 | 1.0 | Specular | The directional bias of reflected and transmitted light resulting in materials appearing rougher or glossier in certain directions. | true | true |
| specular_rotation | float | 0.0 | 0.0 | 1.0 | Specular | Rotation of the axis of specular anisotropy around the surface normal. | true | true |
| transmission | float | 0.0 | 0.0 | 1.0 | Transmission | Transmission of light through the surface for materials such as glass or water. The greater the value the more transparent the material. | true | true |
| transmission_color | color3 | 1, 1, 1 | 0,0,0 | 1,1,1 | Transmission | Color tint on the transmitted light. | true | true |
| transmission_depth | float | 0.0 | 0.0 | 100.0 | Transmission | Specifies the distance light travels inside the material before its becomes exactly the transmission color according to Beer's law. | true | true |
| transmission_scatter | color3 | 0, 0, 0 | 0,0,0 | 1,1,1 | Transmission | Scattering coefficient of the interior medium. Suitable for a large body of liquid or one that is fairly thick, such as an ocean, honey, ice, or frosted glass. | true | true |
| transmission_scatter_anisotropy | float | 0.0 | 0.0 | 1.0 | Transmission | The amount of directional bias, or anisotropy, of the scattering. | true | true |
| transmission_dispersion | float | 0.0 | 0.0 | 100.0 | Transmission | Dispersion amount, describing how much the index of refraction varies across wavelengths. | true | true |
| transmission_extra_roughness | float | 0.0 | 0.0 | 1.0 | Transmission | Additional roughness on top of specular roughness. Positive values blur refractions more than reflections, and negative values blur refractions less. | true | true |
| subsurface | float | 0.0 | 0.0 | 1.0 | Subsurface | The blend between diffuse reflection and subsurface scattering. A value of 1.0 indicates full subsurface scattering and a value 0 for diffuse reflection only. | true | true |
| subsurface_color | color3 | 1, 1, 1 | 0,0,0 | 1,1,1 | Subsurface | The color of the subsurface scattering effect. | true | true |
| subsurface_radius | color3 | 1, 1, 1 | 0,0,0 | 1,1,1 | Subsurface | The mean free path. The distance which light can travel before being scattered inside the surface. | true | true |
| subsurface_scale | float | 1.0 | 0.0 | 10.0 | Subsurface | Scalar weight for the subsurface radius value. | true | true |
| subsurface_anisotropy | float | 0.0 | 0.0 | 1.0 | Subsurface | The direction of subsurface scattering. 0 scatters light evenly, positive values scatter forward and negative values scatter backward. | true | true |
| sheen | float | 0.0 | 0.0 | 1.0 | Sheen | The weight of a sheen layer that can be used to approximate microfibers or fabrics such as velvet and satin. | true | true |
| sheen_color | color3 | 1, 1, 1 | 0,0,0 | 1,1,1 | Sheen | The color of the sheen layer. | true | true |
| sheen_roughness | float | 0.300000011921 | 0.0 | 1.0 | Sheen | The roughness of the sheen layer. | true | true |
| coat | float | 0.0 | 0.0 | 1.0 | Coat | The weight of a reflective clear-coat layer on top of the material. Use for materials such as car paint or an oily layer. |  | true |
| coat_color | color3 | 1, 1, 1 | 0,0,0 | 1,1,1 | Coat | The color of the clear-coat layer's transparency. |  | true |
| coat_roughness | float | 0.10000000149 | 0.0 | 1.0 | Coat | The roughness of the clear-coat reflections. The lower the value, the sharper the reflection. |  | true |
| coat_anisotropy | float | 0.0 | 0.0 | 1.0 | Coat | The amount of directional bias, or anisotropy, of the clear-coat layer. | true | true |
| coat_rotation | float | 0.0 | 0.0 | 1.0 | Coat | The rotation of the anisotropic effect of the clear-coat layer. | true | true |
| coat_IOR | float | 1.5 | 0.0 | 3.0 | Coat | The index of refraction of the clear-coat layer. |  | true |
| coat_normal | vector3 | None | 0,0,0 | 1,1,1 | Coat | Input normal for clear-coat layer |  | true |
| coat_affect_color | float | 0.0 | 0,0,0 | 1,1,1 | Coat | Controls the saturation of diffuse reflection and subsurface scattering below the clear-coat. | true | true |
| coat_affect_roughness | float | 0.0 |  |  | Coat | Controls the roughness of the specular reflection in the layers below the clear-coat. | true | true |
| thin_film_thickness | float | 0.0 | 0.0 | 2000.0 | Thin Film | The thickness of the thin film layer on a surface. Use for materials such as multitone car paint or soap bubbles. | true | true |
| thin_film_IOR | float | 1.5 | 0.0 | 3.0 | Thin Film | The index of refraction of the medium surrounding the material. | true | true |
| emission | float | 0.0 | 0.0 | 1.0 | Emission | The amount of emitted incandescent light. |  | true |
| emission_color | color3 | 1, 1, 1 | 0,0,0 | 1,1,1 | Emission | The color of the emitted light. |  | true |
| opacity | color3 | 1, 1, 1 | 0,0,0 | 1,1,1 | Geometry | The opacity of the entire material. |  | true |
| thin_walled | boolean | False |  |  | Geometry | If tue the surface is double-sided and represents an infinitely thin shell. Suiteable for thin objects such as tree leafs or paper | true | true |
| normal | vector3 | None |  |  | Geometry | Input geometric normal |  | true |
| tangent | vector3 | None |  |  | Geometry | Input geometric tangent |  | true |
