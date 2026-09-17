# Geometry Resources

This directory contains geometry resources used in MaterialX examples, tests, and applications.

## Sphere

`sphere.obj` is a sphere with a diameter of 2 units centered at the origin. It was added to MaterialX in 2019 from PR [#187](https://github.com/AcademySoftwareFoundation/MaterialX/pull/187).

## Cube

`cube.obj` is a 1 cm cube. It's used for UDIM examples and tests. Its 6 faces are mapped to separate UDIM tiles across 1001-1003 and 1011-1013. It was added in 2025 from PR [#2113](https://github.com/AcademySoftwareFoundation/MaterialX/pull/2113) as part of a UDIM example and Graph Editor update.

## Shader Ball

`shaderball.glb` is the default geometry used by the MaterialX Viewer and Graph Editor for previewing materials. The Shader Ball was added in 2019 as `shaderball.obj`, as part of the merge of Autodesk's ShaderX extensions from PR [#187](https://github.com/AcademySoftwareFoundation/MaterialX/pull/187). The current `shaderball.glb` was added in 2022 from PR [#709](https://github.com/AcademySoftwareFoundation/MaterialX/pull/709).

`shaderball_ao.png` is a pre-baked ambient occlusion texture associated with the Shader Ball geometry and used in MaterialXView. It was added in 2019 from commit [6633d5da](https://github.com/AcademySoftwareFoundation/MaterialX/commit/6633d5da121d7641bfda36ddda32fecacf015210), and the Shader Ball mesh was modified for ambient occlusion generation with contributions from Ben Nadler at Lucasfilm.

## Boom Box

`boombox.glb` is the Boom Box sample from the Khronos [glTF-Sample-Assets](https://github.com/KhronosGroup/glTF-Sample-Assets) repository, created and donated by Microsoft under a CC0 license. The glTF PBR shading model example from PR [#870](https://github.com/AcademySoftwareFoundation/MaterialX/pull/870) was contributed by Ed Mackey and added to MaterialX in 2022.

## Chess Set

`chess_set.glb` contains geometry for the Standard Surface chess set example, with geometry and materials contributed by Side Effects. The geometry is from the *Karma: A Beautiful Game* tutorial, with original artwork by Moeen Sayed and Mujtaba Sayed. The chess set example was added to MaterialX in 2022 from PR [#982](https://github.com/AcademySoftwareFoundation/MaterialX/pull/982).

## Cloth

`cloth.obj` is a densely subdivided cloth mesh containing 40,401 vertices and 40,000 faces with corresponding UV coordinates and normals. It was added to MaterialX in 2019 from PR [#198](https://github.com/AcademySoftwareFoundation/MaterialX/pull/198).

## Plane

`plane.obj` is a 1 x 1 plane centered at the origin, subdivided into a 30 x 30 grid. This makes it suitable for per-vertex effects such as displacement. Its geometry was updated from PR [#819](https://github.com/AcademySoftwareFoundation/MaterialX/pull/819) so its UVs aren't flipped in the V direction, removing the need for an extra UV transform.

## Teapot

`teapot.obj` is a sample teapot mesh that was formerly used in MaterialXView as the default geometry when added on May 3, 2019, from PR [#198](https://github.com/AcademySoftwareFoundation/MaterialX/pull/198), and was replaced by the Shader Ball on May 18, 2019 from PR [#215](https://github.com/AcademySoftwareFoundation/MaterialX/pull/215).
