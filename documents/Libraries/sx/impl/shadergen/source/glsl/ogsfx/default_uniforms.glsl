// --------------------------------- Transformation Matrices --------------------------------

// Transform object vertices to world-space
uniform mat4 gWorldXf : World < string UIWidget = "None"; >;

// Transform object normals, tangents, & binormals to world-space
uniform mat4 gWorldITXf : WorldInverseTranspose < string UIWidget = "None"; >;

// Transform object vertices to view space and project them in perspective
uniform mat4 gWvpXf : WorldViewProjection < string UIWidget = "None"; >;

// Provide tranform from 'view' or 'eye' coords back to world-space
uniform mat4 gViewIXf : ViewInverse < string UIWidget = "None"; >;

