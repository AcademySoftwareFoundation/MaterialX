This file is temporary and will be removed before the associated PR is brought out of draft mode.

---

# Notes on Enhancing MaterialX for Nested Dielectrics (A Priority-Based Approach)

### Problem Statement

Complex nested dielectric effects are challenging to represent and render consistently across different software packages. 
For example, a dynamic glass container filled with a vibrant, colored liquid, perhaps ice cubes, carbonation, and condensation, etc.
Current solutions often rely on proprietary solutions, either a shading language like MDL—limiting interoperability and tying assets to specific ecosystems like NVIDIA Omniverse—or else vendor specific properties in a particular DCC.
Within MaterialX there is a need for a portable, renderer-agnostic solution for nested dielectrics to enable broader adoption and asset sharing.

### Objective

Enhance MaterialX to provide robust support for nested dielectrics, enabling the creation of assets with complex graphical qualities that can be rendered consistently across various DCCs and renderers. 
This aligns with MaterialX's core goal of providing a platform-independent description for cross-platform graphics content.

<hr />

### For Discussion: A Priority-Based System

Up for discussion is a priority-based system to handle overlapping dielectric materials, including the following design considerations.

#### Priority Attribute
Add a `priority` attribute (integer) to MaterialX material-semantic nodes.

#### Material Nodes
Evaluate the most suitable nodes for this attribute, considering `surfacematerial`, `volumematerial`, `lightmaterial`, and the top-level `material` node. Create test assets to demonstrate the effect of overlap between each material type.

#### Semantics
The `priority` attribute will define the precedence of an object or material when its dielectric properties overlap with those of other objects. Higher priority objects will "occlude" lower priority objects in terms of refraction and reflection (I.e., if a glass of water has a higher priority than the water inside, the renderer will prioritize the refraction of the glass). This attempts to align with the semantics of OSL.

#### Default Value
Align the default value of zero. A default of 0 (lowest priority) seems reasonable, but the implications for existing MaterialX content should be evaluated. We should also define what happens when no priority is specified. Are there any conflicts introduced if it defaults to zero?

#### Interoperability
The solution should be designed to work across different renderers and avoid renderer-specific features. This will require input from folks on different projects to consider how their renderer will interpret and implement the priority attribute.

#### OpenPBR
Our bias for the overall objective is harmony with OpenUSD and OpenPBR. The solution should defer to the OpenPBR uber-shader model and ideally the priority system can be integrated seamlessly with OpenPBR's existing features. Talk with OpenPBR folks will be much appreciated.

#### Shader Graph
The design should follow the general paradigm of the shader graph, expressing/describing test assets as a set of MaterialX nodes that can be connected together in shader-graph style. A small range of test assets might be useful along with participation from artists to stress-test creation of complex material effects. It should feel MaterialX-y.

#### Consider Implementations
Implementing nested dielectrics with priorities will require some changes to renderers.Some renderers will need to track ray-object relationships and respect the priority attribute during shading. Some renderers behave quite differently. This likely involves modifications to ray traversal and BSDF evaluation logic. We should seek inputs from folks leading these renderers.

#### Naming
The name `priority` has been used in my tests, but it's quite generic. Alternatives like `dielectric_priority` may be preferable.

#### Scope
One open question is whether this attribute should apply only to dielectrics, or whether it should be a more general mechanism that could apply to conflict resolution between other material properties, like opacity and displacement.

<hr />

### General Benefits

#### Platform Independence
Adopting a `priority` attribute on materials should ultimately allow for increased portability and interoperability, on account of the fact that certain qualities are better expressed in proprietary formats. Successfully implementing this change should allow assets to be rendered consistently across different DCCs and renderers, thus facilitating asset sharing and reuse. 

#### Artistic Control
In MaterialX environments, artists will gain more control over the appearance of complex dielectric effects via standards-based means. Today, precisely modeling gaps between overlapping objects takes a high degree of care. In a world of explicit user-defined priority, this may become a bit simplified (in a good way).

#### Future-proofing
Material representation should be standardized for long-term compatibility/interoperability.

<hr />

### Experiments and Inputs

Several private and public experiments have so far been created as exploratory prototypes, with the hope of sparking discussion within the MaterialX community. They are not intended as final implementations but rather as a means to explore different approaches and gather feedback.

#### 1. Focusing on the nodedef [PR #2276: Add priority parameter to dielectric BSDF node](https://github.com/AcademySoftwareFoundation/MaterialX/pull/2276)

This pull request demonstrates adding a `priority` parameter (integer) directly to the `dielectric_bsdf` node. This approach is a starting place based on OSL, but doesn't pay much consideration to MaterialX the way a final proposal probably must. It allows for fine-grained control over the priority of individual dielectric BSDFs within a material.

It reveals the potential for conflicting priorities within the same material if multiple `dielectric_bsdf` nodes have different priority values.

It is also less aligned with the declarative nature of MaterialX, as it mixes material structure with implementation details.

Finally, it may lead to performance issues in renderers that implement nested dielectrics at a low level.

#### 2. Higher-level declaration (Geom) [PR #2287: Add priority attribute to Geometry element (geom)](https://github.com/AcademySoftwareFoundation/MaterialX/pull/2287)

This pull request demonstrates adding a simple integer `priority` attribute to the `Geometry` element (`geom`) in MaterialX. This approach allows priorities to be expressed on a per-object basis. Any dielectric BSDFs on an associated shader would then inherit the priority value from the `Geometry` element. Generators would likely access the value through `MaterialAssign` (by inheritance).

This shows less fine-grained control compared to the `dielectric_bsdf` approach, as all BSDFs on a single object share the same priority. 

<hr />

### Conclusion Community Input and Discussion

Based on preliminary analysis and community feedback, a "per-material" (or potentially per-surfaceshader) attribute appears to be the most promising approach.

We hope the steering group will discuss the addition of a priority attribute to MaterialX material-semantic nodes, enabling object-level control over dielectric interactions.

### Proposed Next Steps

1. Take inputs from the MaterialX TSC
1. Capture the changes in a detailed technical design document and circulate.
1. Submit a prototype implementation via pull request and gather final inputs.
1. Iterate on the PR until a future TSC / acceptance.

---
