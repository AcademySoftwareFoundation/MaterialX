/**
 * WebGPU MaterialXView smoke materials, used by webgpuViewerSmoke.spec.js.
 *
 * Viewer deep-links: index-webgpu.html?file=<mtlx>&geom=<glb>
 * (see MaterialXView/source/index.js and viewer.js).
 */
export const WEBGPU_SMOKE_GEOM = 'Geometry/shaderball.glb';

export const WEBGPU_SMOKE_CASES = [
    {
        id: 'standard_surface_default',
        label: 'Standard Surface Default',
        file: 'Materials/Examples/StandardSurface/standard_surface_default.mtlx',
        rationale: 'basic shadergen and rendering',
    },
    {
        id: 'standard_surface_brick_procedural',
        label: 'Standard Surface Brick Procedural',
        file: 'Materials/Examples/StandardSurface/standard_surface_brick_procedural.mtlx',
        rationale: 'texture loading',
    },
    {
        id: 'standard_surface_marble_solid',
        label: 'Standard Surface Marble Solid',
        file: 'Materials/Examples/StandardSurface/standard_surface_marble_solid.mtlx',
        rationale: 'pattern graphs',
    },
    {
        id: 'standard_surface_carpaint',
        label: 'Standard Surface Carpaint',
        file: 'Materials/Examples/StandardSurface/standard_surface_carpaint.mtlx',
        rationale: 'coat and flakes',
    },
    {
        id: 'open_pbr_soapbubble',
        label: 'Open Pbr Soapbubble',
        file: 'Materials/Examples/OpenPbr/open_pbr_soapbubble.mtlx',
        rationale: 'transmission',
    },
];

export function webgpuViewerUrl(baseURL, materialFile, geom = WEBGPU_SMOKE_GEOM)
{
    const url = new URL('index-webgpu.html', baseURL.endsWith('/') ? baseURL : `${baseURL}/`);
    url.searchParams.set('file', materialFile);
    url.searchParams.set('geom', geom);
    return url.toString();
}
