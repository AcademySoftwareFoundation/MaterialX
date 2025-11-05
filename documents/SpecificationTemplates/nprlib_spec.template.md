# NPR Nodes

NPR nodes support both physically-based and non-physically-based rendering.

### viewdirection

The current scene view direction, as defined by the shading environment. 

@MX_TABLE_viewdirection@

The view direction is a normalized vector from the camera/eye, to visible objects in the scene. In a PBR shading context, this would only include the primary ray direction, and not any secondary/reflection rays.

### facingratio

The geometric facing ratio of surface normals and the view direction.

@MX_TABLE_facingratio@

Facing ratio is computed as the dot product between the view direction and geometric normal. Output values of 1.0 are facing towards the view direction, while values of -1.0 are facing away. Values of 0 are surface normals perpendicular to the view direction.

### gooch_shade

Computes the color from single-pass shading portion of the Gooch[^Gooch1998] lighting model.

@MX_TABLE_gooch_shade@

Gooch shade provides an illustrative shading effect by blending colors based on the angle between the surface normal and the light direction. It also provides a very simple specular highlight, on top of the warm and cool colors.

The `warm_color` input corresponds to $C_w$ in the model, and represents colors facing towards the `light_direction`.

The `cool_color` input corresponds to $C_c$, and represents colors facing away from the `light_direction`.

The `specular_intensity` input corresponds to $N$, and represents the intensity of the highlight.

The `shininess` input corresponds to $H$, and represents the size of the highlight across surface.

The `light_direction` input corresponds to $L$, and represents the world space direction of the light.

### Gooch Equations

$$M = \frac{1+({N} \cdot {L})}{2}$$

$$D = (1-{M}) * {C_w} + {M} * {C_c}$$

$$E = reflect({V},{N})$$

$$S = ( max(E \cdot -{L}, 0 )^{H} ) * {N}$$

$$out = {D} + {S}$$

# References

[^Gooch1998]: Gooch et al., **A Non-Photorealistic Lighting Model For Automatic Technical Illustration**, <https://users.cs.northwestern.edu/~ago820/SIG98/gooch98.pdf>, 1998.