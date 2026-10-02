//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

/**
 * WGSL reflection manifest builder.
 *
 * The in-repo `WgslShaderGenerator` emits WGSL but does not emit the JSON reflection
 * manifest that the TSL bridge (`mxtsladapter.js`) consumes. Rather than reintroduce
 * manifest emission into the C++ generator, we reconstruct the manifest here in JS from
 * two sources that are already available after `generate()`:
 *
 *   1. The generated WGSL text — supplies the `@group/@binding` layout, the final WGSL
 *      type spellings, the entry-function names, and the vertex-input struct (varyings).
 *   2. The `Shader` object — supplies what is NOT present in the WGSL text: each uniform's
 *      default value (`ShaderPort.getValue()`) and the public-vs-host role (the uniform
 *      block name: `PublicUniforms` => editable "uniform", everything else => "host").
 *
 * The output matches the "bindings" reflection shape accepted by
 * `normalizeReflection()` in mxtsladapter.js, so no adapter changes are required.
 */

// Uniform block names emitted by HwShaderGenerator (see source/MaterialXGenHw/HwConstants.cpp).
// The role split mirrors the original C++ manifest: PublicUniforms members are the
// user-editable surface inputs; all other blocks are engine/host-set.
const PUBLIC_UNIFORMS = 'PublicUniforms';
const LIGHT_DATA = 'LightData';

// Entry-function names emitted by WgslShaderGenerator::emitPixelStage / emitVertexStage
// (see source/MaterialXGenWgsl/WgslShaderGenerator.cpp). The adapter is entry-name
// agnostic (it reads manifest.entry), so emitting the in-repo names is sufficient.
const PIXEL_ENTRY = 'fragmentMain';
const VERTEX_ENTRY = 'vertexMain';

/**
 * Format a MaterialX port value as a JS scalar / array / boolean.
 * Mirrors the C++ `valueToJson` in WgslShaderGenerator.cpp so the produced object graph
 * is identical to what `JSON.parse(getWgslGeneratedManifest(...))` used to yield.
 *
 * @param {Object} port - A bound mx.ShaderPort, or null.
 * @return {(number|boolean|number[]|null)} The default value, or null when unset.
 */
function portValueToJson( port ) {

	const value = port && port.getValue && port.getValue();
	if ( ! value ) return null;

	const s = value.getValueString();
	if ( s.indexOf( ',' ) !== - 1 ) {

		// Vector / color / matrix: comma-separated numbers.
		return s.split( ',' )
			.map( ( item ) => item.trim() )
			.filter( ( item ) => item.length > 0 )
			.map( ( item ) => parseFloat( item ) );

	}
	if ( s === 'true' ) return true;
	if ( s === 'false' ) return false;
	if ( s === '' ) return 0;
	const n = Number( s );
	return Number.isNaN( n ) ? s : n;

}

/**
 * Build a name -> { role, port } map from a shader stage's uniform blocks.
 * The block name determines the role; LightData members are not enumerated individually
 * (the light-data array is exposed as a single `var<uniform>` binding instead).
 *
 * @param {Object} shader - The mx.Shader object.
 * @param {string} stageName - 'pixel' or 'vertex'.
 * @return {Map<string, {role: string, port: Object}>}
 */
function collectUniformPorts( shader, stageName ) {

	const map = new Map();
	let stage;
	try {

		stage = shader.getStage( stageName );

	} catch ( e ) {

		return map;

	}
	if ( ! stage ) return map;

	const blocks = stage.getUniformBlocks(); // JS object keyed by block name
	for ( const blockName of Object.keys( blocks ) ) {

		if ( blockName === LIGHT_DATA ) continue;
		const block = blocks[ blockName ];
		if ( ! block ) continue;
		const role = blockName === PUBLIC_UNIFORMS ? 'uniform' : 'host';
		const count = block.size();
		for ( let i = 0; i < count; ++ i ) {

			const port = block.get( i );
			if ( ! port ) continue;
			const varName = port.getVariable();
			map.set( varName, { role, port } );
			// The C++ binding returns struct-qualified names (e.g. "u_pub.base")
			// but parseBindings looks up bare struct member names ("base").
			// Store both forms so the lookup succeeds either way.
			const dotIdx = varName.lastIndexOf( '.' );
			if ( dotIdx >= 0 ) map.set( varName.slice( dotIdx + 1 ), { role, port } );

		}

	}

	return map;

}

// Matches `@group(N) @binding(M) var[<addr>] name: type;` lines emitted by
// WgslResourceBindingContext. `var<uniform>` covers struct UBOs (PublicUniforms,
// PrivateUniforms) and light-data arrays; bare `var` covers texture_2d<f32> and sampler.
const BINDING_RE = /@group\(\s*(\d+)\s*\)\s*@binding\(\s*(\d+)\s*\)\s*var(?:<[^>]*>)?\s+([A-Za-z_]\w*)\s*:\s*([^;]+);/g;

// Matches a struct definition `struct Name { member: type, ... }` and captures
// the body. Used to expand struct-type uniform bindings into per-member entries.
const STRUCT_DEF_RE = /\bstruct\s+(\w+)\s*\{([^}]*)\}/g;

/**
 * Parse struct definitions from WGSL text.
 * @return {Map<string, Array<{name: string, type: string}>>}
 */
function parseStructDefs( wgsl ) {

	const structs = new Map();
	STRUCT_DEF_RE.lastIndex = 0;
	for ( let m = STRUCT_DEF_RE.exec( wgsl ); m !== null; m = STRUCT_DEF_RE.exec( wgsl ) ) {

		const sName = m[ 1 ];
		const members = [];
		for ( const rawLine of m[ 2 ].split( '\n' ) ) {

			const line = rawLine.trim().replace( /,$/, '' );
			if ( ! line ) continue;
			const colon = line.indexOf( ':' );
			if ( colon < 0 ) continue;
			const name = line.slice( 0, colon ).trim();
			const type = line.slice( colon + 1 ).trim();
			if ( name && type ) members.push( { name, type } );

		}
		if ( members.length ) structs.set( sName, members );

	}
	return structs;

}

/**
 * Parse the `@group/@binding` resource declarations out of the WGSL text into manifest
 * bindings, attaching role + default value from the uniform-port map.
 *
 * Struct-type uniform bindings (PublicUniforms, PrivateUniforms) are expanded into
 * per-member entries so the adapter sees the same flat list it did before struct packing.
 *
 * @param {string} wgsl - WGSL source text for one stage.
 * @param {Map} portMap - Uniform-port map from collectUniformPorts().
 * @param {string} [stageName='pixel'] - 'pixel' or 'vertex'; tags each returned binding.
 */
function parseBindings( wgsl, portMap, stageName ) {

	stageName = stageName || 'pixel';
	const structDefs = parseStructDefs( wgsl );
	const bindings = [];
	BINDING_RE.lastIndex = 0;
	for ( let m = BINDING_RE.exec( wgsl ); m !== null; m = BINDING_RE.exec( wgsl ) ) {

		const group = Number( m[ 1 ] );
		const binding = Number( m[ 2 ] );
		const name = m[ 3 ];
		const type = m[ 4 ].trim();

		if ( name.endsWith( '_texture' ) || type.startsWith( 'texture_' ) ) {

			bindings.push( { stage: stageName, group, binding, name, type, role: 'texture', key: name.replace( /_texture$/, '' ) } );

		} else if ( name.endsWith( '_sampler' ) || type === 'sampler' ) {

			bindings.push( { stage: stageName, group, binding, name, type, role: 'sampler', key: name.replace( /_sampler$/, '' ) } );

		} else if ( type.startsWith( 'array<' ) ) {

			bindings.push( { stage: stageName, group, binding, name, type, role: 'lightData' } );

		} else if ( structDefs.has( type ) ) {

			const members = structDefs.get( type );
			for ( const member of members ) {

				const info = portMap.get( member.name );
				const role = info ? info.role : 'host';
				const value = info ? portValueToJson( info.port ) : null;
				bindings.push( {
					stage: stageName, group, binding, name: member.name, type: member.type,
					role, value, structInstance: name, structType: type
				} );

			}

		} else {

			const info = portMap.get( name );
			const role = info ? info.role : 'host';
			const value = info ? portValueToJson( info.port ) : null;
			bindings.push( { stage: stageName, group, binding, name, type, role, value } );

		}

	}

	return bindings;

}

/**
 * Parse the pixel entry signature `fn FragmentMain(<inst>: <Struct>) -> vec4f` plus the
 * referenced `struct <Struct> { name: type, ... }` into a single entryParam carrying the
 * varying members. normalizeReflection() flattens this into the varying input list.
 */
function parseEntryParams( wgsl ) {

	const sig = new RegExp( `\\bfn\\s+${ PIXEL_ENTRY }\\s*\\(\\s*([A-Za-z_]\\w*)\\s*:\\s*([A-Za-z_]\\w*)\\s*\\)` ).exec( wgsl );
	if ( ! sig ) return [];

	const instName = sig[ 1 ];
	const structName = sig[ 2 ];

	const structRe = new RegExp( `\\bstruct\\s+${ structName }\\s*\\{([^}]*)\\}` );
	const structMatch = structRe.exec( wgsl );
	const members = [];
	if ( structMatch ) {

		for ( const rawLine of structMatch[ 1 ].split( '\n' ) ) {

			const line = rawLine.trim().replace( /,$/, '' );
			if ( ! line ) continue;
			const colon = line.indexOf( ':' );
			if ( colon < 0 ) continue;
			const decl = line.slice( 0, colon );
			const type = line.slice( colon + 1 ).trim();
			// Skip the @builtin(position) member: it is the fragment-position builtin, not a
			// MaterialX surface varying — the library/material code never reads it, and binding
			// it would mis-map to a varying semantic with a vec4/vec3 type mismatch.
			if ( /@builtin\b/.test( decl ) ) continue;
			// Drop @location(...) / @interpolate(flat) attribute prefixes; keep the bare
			// member identifier (the generator emits attributes inline in the struct).
			const name = decl.replace( /@\w+\s*\([^)]*\)/g, '' ).trim();
			if ( name && type ) members.push( { name, type } );

		}

	}

	return [ { name: instName, type: structName, members } ];

}

/**
 * Parse `struct VertexInputs { @location(N) name: type, ... }` from vertex WGSL.
 * Returns an array of { name, type, location } for each vertex attribute.
 */
function parseVertexInputs( vertexWgsl ) {

	if ( ! vertexWgsl ) return [];
	const structRe = /\bstruct\s+VertexInputs\s*\{([^}]*)\}/;
	const structMatch = structRe.exec( vertexWgsl );
	if ( ! structMatch ) return [];

	const inputs = [];
	for ( const rawLine of structMatch[ 1 ].split( '\n' ) ) {

		const line = rawLine.trim().replace( /,$/, '' );
		if ( ! line ) continue;
		const colon = line.indexOf( ':' );
		if ( colon < 0 ) continue;
		const decl = line.slice( 0, colon );
		const type = line.slice( colon + 1 ).trim();
		const locMatch = /@location\s*\(\s*(\d+)\s*\)/.exec( decl );
		const name = decl.replace( /@\w+\s*\([^)]*\)/g, '' ).trim();
		if ( name && type ) {

			inputs.push( { name, type, location: locMatch ? Number( locMatch[ 1 ] ) : inputs.length } );

		}

	}
	return inputs;

}

/**
 * Parse `struct VertexData { @builtin(position) clipPosition: vec4f, @location(N) name: type, ... }`
 * from vertex WGSL. Returns an array of { name, type, location } for each non-builtin output.
 */
function parseVertexOutputs( vertexWgsl ) {

	if ( ! vertexWgsl ) return [];
	const structRe = /\bstruct\s+VertexData\s*\{([^}]*)\}/;
	const structMatch = structRe.exec( vertexWgsl );
	if ( ! structMatch ) return [];

	const outputs = [];
	for ( const rawLine of structMatch[ 1 ].split( '\n' ) ) {

		const line = rawLine.trim().replace( /,$/, '' );
		if ( ! line ) continue;
		const colon = line.indexOf( ':' );
		if ( colon < 0 ) continue;
		const decl = line.slice( 0, colon );
		const type = line.slice( colon + 1 ).trim();
		if ( /@builtin\b/.test( decl ) ) continue;
		const locMatch = /@location\s*\(\s*(\d+)\s*\)/.exec( decl );
		const name = decl.replace( /@\w+\s*\([^)]*\)/g, '' ).trim();
		if ( name && type ) {

			outputs.push( { name, type, location: locMatch ? Number( locMatch[ 1 ] ) : outputs.length } );

		}

	}
	return outputs;

}

/**
 * Build the WGSL reflection manifest (bindings format) from a generated Shader + its WGSL.
 *
 * When both vertex and pixel WGSL are supplied, the manifest includes vertex-stage
 * reflection (inputs, outputs, bindings) so the TSL bridge can wire both stages.
 *
 * @param {Object} shader - The mx.Shader returned by WgslShaderGenerator.generate().
 * @param {string} pixelWgsl - The generated pixel-stage WGSL source.
 * @param {string} [vertexWgsl] - The generated vertex-stage WGSL source (optional).
 * @return {Object} Manifest consumable by normalizeReflection() / convertToTslPortable().
 */
export function buildWgslManifest( shader, pixelWgsl, vertexWgsl ) {

	const pixelPortMap = collectUniformPorts( shader, 'pixel' );
	const pixelBindings = parseBindings( pixelWgsl, pixelPortMap, 'pixel' );

	const result = {
		entry: { vertex: VERTEX_ENTRY, pixel: PIXEL_ENTRY },
		output: 'vec4f',
		entryParams: parseEntryParams( pixelWgsl ),
		bindings: pixelBindings
	};

	if ( vertexWgsl ) {

		const vertexPortMap = collectUniformPorts( shader, 'vertex' );
		result.vertexBindings = parseBindings( vertexWgsl, vertexPortMap, 'vertex' );
		result.vertexInputs = parseVertexInputs( vertexWgsl );
		result.vertexOutputs = parseVertexOutputs( vertexWgsl );
		result.vertexWgsl = vertexWgsl;

	}

	return result;

}
