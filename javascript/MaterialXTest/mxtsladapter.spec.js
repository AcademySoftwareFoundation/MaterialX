import { test, expect } from '@playwright/test';
import { convertVertexToTslPortable, normalizeReflection } from '../MaterialXView/source/mxtsladapter.js';

/**
 * Minimal vertex WGSL matching the shape emitted by WgslShaderGenerator.
 * Stripped to the bare essentials so tests exercise rewriting without
 * depending on a full material.
 */
const VERTEX_WGSL = `\
struct PrivateUniforms
{
    u_worldMatrix: mat4x4f,
    u_viewProjectionMatrix: mat4x4f,
    u_worldInverseTransposeMatrix: mat4x4f
}
@group(0) @binding(0) var<uniform> u_prv: PrivateUniforms;

struct VertexInputs
{
    @location(0) i_position: vec3f,
    @location(1) i_normal: vec3f,
    @location(2) i_tangent: vec3f,
}
struct VertexData
{
    @builtin(position) clipPosition: vec4f,
    @location(0) normalWorld: vec3f,
    @location(1) tangentWorld: vec3f,
    @location(2) positionWorld: vec3f,
}

@vertex
fn vertexMain(vsIn: VertexInputs) -> VertexData
{
    var i_position = vsIn.i_position;
    var i_normal = vsIn.i_normal;
    var i_tangent = vsIn.i_tangent;
    var hPositionWorld: vec4f = u_prv.u_worldMatrix * vec4f(i_position, 1.0);
    var vd: VertexData;
    vd.clipPosition = u_prv.u_viewProjectionMatrix * hPositionWorld;
    vd.normalWorld = normalize((u_prv.u_worldInverseTransposeMatrix * vec4f(i_normal, 0.0)).xyz);
    vd.tangentWorld = normalize((u_prv.u_worldMatrix * vec4f(i_tangent, 0.0)).xyz);
    vd.positionWorld = hPositionWorld.xyz;
    return vd;
}
`;

/**
 * Manifest that mirrors what buildWgslManifest produces for the vertex WGSL above.
 */
const MANIFEST = {
	entry: 'fragmentMain',
	vertexEntry: 'vertexMain',
	output: 'vec4f',
	bindings: [],
	vertexBindings: [
		{ group: 0, binding: 0, name: 'u_prv', type: 'PrivateUniforms', stage: 'vertex' }
	],
	vertexInputs: [
		{ name: 'i_position', type: 'vec3f', location: 0 },
		{ name: 'i_normal', type: 'vec3f', location: 1 },
		{ name: 'i_tangent', type: 'vec3f', location: 2 }
	],
	vertexOutputs: [
		{ name: 'normalWorld', type: 'vec3f', location: 0 },
		{ name: 'tangentWorld', type: 'vec3f', location: 1 },
		{ name: 'positionWorld', type: 'vec3f', location: 2 }
	],
	vertexWgsl: VERTEX_WGSL
};

test.describe( 'normalizeReflection', () => {

	test( 'adds vertex:host semantic to vertex bindings', () => {

		const norm = normalizeReflection( MANIFEST );
		expect( norm.vertexUniforms ).toBeDefined();
		expect( norm.vertexUniforms.length ).toBeGreaterThan( 0 );
		for ( const u of norm.vertexUniforms ) {

			expect( u.semantic ).toBe( 'vertex:host' );

		}

	} );

	test( 'adds attribute semantics to vertex inputs', () => {

		const norm = normalizeReflection( MANIFEST );
		expect( norm.vertexInputs ).toBeDefined();
		for ( const vi of norm.vertexInputs ) {

			expect( vi.semantic ).toMatch( /^attribute:/ );
			expect( vi.semantic ).toBe( `attribute:${ vi.name }` );

		}

	} );

	test( 'preserves vertexOutputs with varying semantics', () => {

		const norm = normalizeReflection( MANIFEST );
		expect( norm.vertexOutputs ).toBeDefined();
		expect( norm.vertexOutputs.length ).toBe( 3 );
		for ( const vo of norm.vertexOutputs ) {

			expect( vo.semantic ).toMatch( /^varying:/ );

		}

	} );

} );

test.describe( 'convertVertexToTslPortable', () => {

	test( 'returns entry with flat parameters', () => {

		const result = convertVertexToTslPortable( VERTEX_WGSL, MANIFEST );
		expect( result.name ).toBe( 'vertexMain' );
		expect( result.entry ).toMatch( /fn vertexMain\(/ );
		expect( result.entry ).toContain( 'i_position: vec3f' );
		expect( result.entry ).toContain( 'i_normal: vec3f' );
		expect( result.entry ).toContain( 'i_tangent: vec3f' );

	} );

	test( 'flattens u_prv. struct access', () => {

		const result = convertVertexToTslPortable( VERTEX_WGSL, MANIFEST );
		expect( result.entry ).not.toContain( 'u_prv.' );
		expect( result.entry ).toContain( 'u_worldMatrix' );
		expect( result.entry ).toContain( 'u_viewProjectionMatrix' );

	} );

	test( 'flattens vsIn. struct access', () => {

		const result = convertVertexToTslPortable( VERTEX_WGSL, MANIFEST );
		expect( result.entry ).not.toContain( 'vsIn.' );

	} );

	test( 'rewrites vd.* to varyings.* for varying outputs', () => {

		const result = convertVertexToTslPortable( VERTEX_WGSL, MANIFEST );
		expect( result.entry ).not.toMatch( /\bvd\.normalWorld\b/ );
		expect( result.entry ).not.toMatch( /\bvd\.tangentWorld\b/ );
		expect( result.entry ).not.toMatch( /\bvd\.positionWorld\b/ );
		expect( result.entry ).toContain( 'varyings.normalWorld' );
		expect( result.entry ).toContain( 'varyings.tangentWorld' );
		expect( result.entry ).toContain( 'varyings.positionWorld' );

	} );

	test( 'rewrites vd.clipPosition to mx_clipPosition', () => {

		const result = convertVertexToTslPortable( VERTEX_WGSL, MANIFEST );
		expect( result.entry ).not.toMatch( /\bvd\.clipPosition\b/ );
		expect( result.entry ).toContain( 'mx_clipPosition' );

	} );

	test( 'replaces VertexData var decl with vec4f', () => {

		const result = convertVertexToTslPortable( VERTEX_WGSL, MANIFEST );
		expect( result.entry ).not.toContain( 'var vd: VertexData' );
		expect( result.entry ).toContain( 'var mx_clipPosition: vec4f' );

	} );

	test( 'returns vec4f (clip position)', () => {

		const result = convertVertexToTslPortable( VERTEX_WGSL, MANIFEST );
		expect( result.entry ).toContain( 'return mx_clipPosition' );
		expect( result.entry ).toMatch( /-> vec4f/ );

	} );

	test( 'strips VertexInputs and VertexData structs from includes', () => {

		const result = convertVertexToTslPortable( VERTEX_WGSL, MANIFEST );
		expect( result.includes ).not.toContain( 'struct VertexInputs' );
		expect( result.includes ).not.toContain( 'struct VertexData' );

	} );

	test( 'strips @group/@binding declarations from includes', () => {

		const result = convertVertexToTslPortable( VERTEX_WGSL, MANIFEST );
		expect( result.includes ).not.toMatch( /@group\(/ );
		expect( result.includes ).not.toMatch( /@binding\(/ );

	} );

	test( 'params contain vertex inputs with attribute semantics', () => {

		const result = convertVertexToTslPortable( VERTEX_WGSL, MANIFEST );
		const inputParams = result.params.filter( p => p.semantic && p.semantic.startsWith( 'attribute:' ) );
		expect( inputParams.length ).toBe( 3 );
		expect( inputParams.map( p => p.name ) ).toEqual( [ 'i_position', 'i_normal', 'i_tangent' ] );

	} );

	test( 'params contain vertex uniforms with vertex:host semantic', () => {

		const result = convertVertexToTslPortable( VERTEX_WGSL, MANIFEST );
		const uniformParams = result.params.filter( p => p.semantic === 'vertex:host' );
		expect( uniformParams.length ).toBeGreaterThan( 0 );
		const names = uniformParams.map( p => p.name );
		expect( names ).toContain( 'u_worldMatrix' );
		expect( names ).toContain( 'u_viewProjectionMatrix' );
		expect( names ).toContain( 'u_worldInverseTransposeMatrix' );

	} );

} );
