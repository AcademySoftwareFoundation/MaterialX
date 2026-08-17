# Geometry Resources

This directory contains geometry resources used in MaterialX examples, tests, and applications.

## Sphere

`sphere.obj` is a sphere with a diameter of 2 units centered at the origin.

## Cube

`cube.obj` is a 1 cm cube. It's used for UDIM examples and tests. Its 6 faces are mapped to separate UDIM tiles across 1001-1003 and 1011-1013.

## Shaderball

`shaderball.glb` is the default geometry used by the MaterialX Viewer and Graph Editor for previewing materials. The Shader Ball was added in 2019 as `shaderball.obj`, as part of the merge of Autodesk's ShaderX extensions.

## Boombox

`boombox.glb` is a glTF example from Ed Mackey. It was added in 2022 as an example for the glTF PBR shading model.

## Chess Set

`chess_set.glb` contains a chess set geometry from the *Karma: A Beautiful Game* tutorial, with original artwork and assets by Moeen Sayed and Mujtaba Sayed. It's used with the Standard Surface chess set example, which provides materials and texture maps for the chessboard and individual chess pieces.

## Cloth

`cloth.obj` is a densely subdivided cloth mesh containing 40,401 vertices and 40,000 faces with corresponding UV coordinates and normals.

## Plane

`plane.obj` is a 1 x 1 plane centered at the origin. Its geometry was updated so its UVs aren't flipped in the V direction, removing the need for an extra UV transform.

## Teapot

`teapot.obj` is a sample teapot mesh that was formerly used in MaterialXView as the default geometry.
