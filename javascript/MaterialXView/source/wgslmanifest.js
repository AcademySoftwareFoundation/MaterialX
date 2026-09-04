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
 * Build a name -> { role, port } map from the pixel stage's uniform blocks.
 * The block name determines the role; LightData members are not enumerated individually
 * (the light-data array is exposed as a single `var<uniform>` binding instead).
 */
function collectUniformPorts( shader ) {

	const map = new Map();
	let stage;
	try {

		stage = shader.getStage( 'pixel' );

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
			map.set( port.getVariable(), { role, port } );

		}

	}

	return map;

}

// Matches `@group(N) @binding(M) var[<addr>] name: type;` lines emitted by
// WgslResourceBindingContext. `var<uniform>` covers scalar/vector/matrix/array uniforms;
// bare `var` covers texture_2d<f32> and sampler bindings.
const BINDING_RE = /@group\(\s*(\d+)\s*\)\s*@binding\(\s*(\d+)\s*\)\s*var(?:<[^>]*>)?\s+([A-Za-z_]\w*)\s*:\s*([^;]+);/g;

/**
 * Parse the `@group/@binding` resource declarations out of the WGSL text into manifest
 * bindings, attaching role + default value from the uniform-port map.
 */
function parseBindings( wgsl, portMap ) {

	const bindings = [];
	BINDING_RE.lastIndex = 0;
	for ( let m = BINDING_RE.exec( wgsl ); m !== null; m = BINDING_RE.exec( wgsl ) ) {

		const group = Number( m[ 1 ] );
		const binding = Number( m[ 2 ] );
		const name = m[ 3 ];
		const type = m[ 4 ].trim();

		if ( name.endsWith( '_texture' ) || type.startsWith( 'texture_' ) ) {

			bindings.push( { stage: 'pixel', group, binding, name, type, role: 'texture', key: name.replace( /_texture$/, '' ) } );

		} else if ( name.endsWith( '_sampler' ) || type === 'sampler' ) {

			bindings.push( { stage: 'pixel', group, binding, name, type, role: 'sampler', key: name.replace( /_sampler$/, '' ) } );

		} else if ( type.startsWith( 'array<' ) ) {

			// Light-data array (struct array). The adapter re-parses the struct itself,
			// so no per-member value is needed here.
			bindings.push( { stage: 'pixel', group, binding, name, type, role: 'lightData' } );

		} else {

			const info = portMap.get( name );
			const role = info ? info.role : 'host';
			const value = info ? portValueToJson( info.port ) : null;
			bindings.push( { stage: 'pixel', group, binding, name, type, role, value } );

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
 * Build the WGSL reflection manifest (bindings format) from a generated Shader + its WGSL.
 *
 * @param {Object} shader - The mx.Shader returned by WgslShaderGenerator.generate().
 * @param {string} wgsl - The generated pixel-stage WGSL source.
 * @return {Object} Manifest consumable by normalizeReflection() / convertToTslPortable().
 */
export function buildWgslManifest( shader, wgsl ) {

	const portMap = collectUniformPorts( shader );
	return {
		entry: { vertex: VERTEX_ENTRY, pixel: PIXEL_ENTRY },
		output: 'vec4f',
		entryParams: parseEntryParams( wgsl ),
		bindings: parseBindings( wgsl, portMap )
	};

}
