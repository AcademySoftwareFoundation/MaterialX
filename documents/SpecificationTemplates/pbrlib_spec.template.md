# pbrlib

### oren_nayar_diffuse_bsdf
Constructs a diffuse reflection BSDF based on the Oren-Nayar reflectance model.

@MX_TABLE_oren_nayar_diffuse_bsdf@
                 |

The `color` input represents the diffuse albedo, and corresponds to $\rho$ (rho) in the Oren-Nayar model.

The `roughness` input corresponds to $\sigma$ (sigma) in the Oren-Nayar model, and a `roughness` of 0.0 produces Lambertian reflectance.

The `energy_compensation` input selects between the Qualitative Oren-Nayar[^Oren1994] and Energy-Preserving Oren-Nayar[^Portsmouth2025] models of diffuse reflectance.

### Qualitative Oren-Nayar Reflectance Equations

$$A=1-0.5\left(\frac{\sigma^2}{\sigma^2+0.33}\right)$$

$$B=1-0.45\left(\frac{\sigma^2}{\sigma^2+0.09}\right)$$

$$g(\omega_i,\omega_o)=\cos_{(\theta_i-\theta_o)}\sin_{{max}(\theta_i,\theta_o)}\tan_{{min}(\theta_i,\theta_o)}$$

$$L_r(\omega_i,\omega_o)=\frac{\rho}{\pi}(A+Bg(\omega_i,\omega_o))$$

### dielectric_bsdf
Constructs a reflection and/or transmission BSDF based on a microfacet model and a Fresnel curve for dielectrics[^Walter2007].

@MX_TABLE_dielectric_bsdf@

The `tint` input corresponds to $t$ in the dielectric model, and represents a non-physical color weight to tint the reflected and transmitted light.

The `ior` input corresponds to $\eta$ (eta) in the dielectric model, and represents the real-valued index of refraction of the surface.

The `scatter_mode` input selects between `R` (reflection only), `T` (transmission only), and `RT` (both reflection and transmision).

If reflection scattering is enabled, this node may be layered vertically over a base BSDF for the surface beneath the dielectric layer. By chaining multiple `dielectric_bsdf` nodes you can describe a surface with multiple specular lobes.

If transmission scattering is enabled, this node may be layered over a VDF describing the surface interior to handle absorption and scattering inside the medium, useful for colored glass, turbid water, etc.

#### Dielectric Equations

These equations are based on a standard optimized implementation[^Lagarde2013], rather than the original paper[^Walter2007].

$$c=\cos_{\theta}$$

$$g=\sqrt{\eta^2+c^2-1}$$

$$R=t\frac{(g-c)^2}{2(g-c)^2}\left(1+\frac{(c(g+c)-1)^2}{(c(g-c)+1)^2}\right)$$

### generalized_schlick_bsdf
Constructs a reflection and/or transmission BSDF based on a microfacet model and a generalized Schlick Fresnel curve[^Hoffman2023].

@MX_TABLE_generalized_schlick_bsdf@

The `color0` and `color90` inputs correspond to $r_0$ and $r_{90}$ in the generalized Schlick model.

The `color82` input corresponds to $t$ in the Generalized Schlick model.

The `exponent` input is the exponent for Schlick blending between `color0` and `color90`, and corresponds to $\alpha$ in the generalized Schlick model.

The `scatter_mode` input selects between `R` (reflection only), `T` (transmission only), and `RT` (both reflection and transmision).

If reflection scattering is enabled, this node may be layered vertically over a base BSDF for the surface beneath the generalized Schlick layer. By chaining multiple `generalized_schlick_bsdf` nodes you can describe a surface with multiple specular lobes.

If transmission scattering is enabled, this node may be layered over a VDF describing the surface interior to handle absorption and scattering inside the medium, useful for colored glass, turbid water, etc.

#### Generalized Schlick Equations

$$\theta_{max}=\arccos(\frac{1}{7})$$

$$a=\frac{(r_0+(r_{90}-r_0))(1-\cos_{\theta_{max}})^\alpha(1-t)}{\cos_{\theta_{max}}(1-\cos_{\theta_{max}})^6}$$

$$F_\theta=(r_0+(r_{90}-r_0))(1-\cos_{\theta})^\alpha-a\cos_{\theta}(1-\cos_{\theta})^6$$

### sheen_bsdf
Constructs a microfacet BSDF for the back-scattering properties of cloth-like materials.

@MX_TABLE_sheen_bsdf@

The `color` input corresponds to $c$ in the sheen model, and represents a non-physical color tint on the sheen lobe.

The `roughness` input corresponds to $r$ in the sheen model, and represents the degree to which the microfibers diverge from the normal direction.

The `mode` option selects between two available sheen models, Conty-Kulla[^Conty2017] and Zeltner[^Zeltner2022].

This node may be layered vertically over a base BSDF for the surface beneath the sheen layer. 

#### Conty-Kulla Sheen Equations

$$D_\theta=\frac{(2 + \frac{1}{r})(1 - {\cos_\theta}^2)^\frac{0.5}{r}}{2\pi}$$

$$L_r(\omega_i,\omega_o,\omega_h)=\frac{cD(\theta_h)}{cos{\theta_i}+cos{\theta_o}-cos{\theta_i}cos{\theta_o}}$$

# References

[^Conty2017]: Alejandro Conty, Christopher Kulla, **Production Friendly Microfacet Sheen BRDF**, <https://fpsunflower.github.io/ckulla/data/s2017_pbs_imageworks_sheen.pdf>, 2017

[^Hoffman2023]: Naty Hoffman, **Generalization of Adobe's Fresnel Model**, <https://renderwonk.com/publications/wp-generalization-adobe/gen-adobe.pdf>, 2023

[^Lagarde2013]: Sébastien Lagarde, **Memo on Fresnel equations**, https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/, 2013

[^Oren1994]: Michael Oren, Shree K. Nayar, **Generalization of Lambert’s Reflectance Model**, <https://dl.acm.org/doi/10.1145/192161.192213>, 1994

[^Portsmouth2025]: Portsmouth et al., **EON: A practical energy-preserving rough diffuse BRDF**, <https://www.jcgt.org/published/0014/01/06/>, 2025.

[^Walter2007]: Bruce Walter et al., **Microfacet Models for Refraction through Rough Surfaces**, <https://www.graphics.cornell.edu/~bjw/microfacetbsdf.pdf>, 2007

[^Zeltner2022]: Tizian Zeltner et al., **Practical Multiple-Scattering Sheen Using Linearly Transformed Cosines**, <https://tizianzeltner.com/projects/Zeltner2022Practical/>, 2022