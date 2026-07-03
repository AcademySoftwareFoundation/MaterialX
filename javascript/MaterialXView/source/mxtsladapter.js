/**
 * mxtsladapter
 *
 * Adapts MaterialX-generated WGSL into Three.js TSL / WebGPU.
 *
 * The MaterialX `WgslShaderGenerator` emits a complete, standard WGSL module:
 * helper structs + functions, a uniform block + texture/sampler bindings declared
 * with `@group(N) @binding(M)`, and a single material entry function that returns
 * the surface result. Three.js never ingests such a module directly — its only raw
 * WGSL entry point is `wgslFn`, which accepts exactly one function whose resources
 * arrive as *parameters* (TSL owns the bindings).
 *
 * This module performs that reshaping ("TSL-portable WGSL"):
 *   - `convertToTslPortable(wgsl, manifest)` rewrites the entry function so that the
 *     uniform-block members and texture/sampler bindings become explicit parameters,
 *     and strips the now-unused `@group/@binding` declarations and uniform struct.
 *     Everything else (helper structs/functions) is returned verbatim as `includes`.
 *   - `createMxWgslMaterial({ THREE, TSL, ... })` wires the converted entry up
 *     with `wgslFn`, binding each parameter to the matching TSL node
 *     (`uniform`/`texture`/`sampler`/builtin), and returns a NodeMaterial.
 *
 * The manifest is emitted alongside the WGSL by the generator and makes the
 * conversion deterministic (no fragile parsing of binding layouts).
 */

// ── WGSL text utilities ─────────────────────────────────────────────────────

function skipWs( src, i ) {

	while ( i < src.length && /\s/.test( src[ i ] ) ) i ++;
	return i;

}

// Escape a string for safe use as a literal inside a RegExp. The function/resource names
// here are generator-emitted WGSL identifiers, but escaping keeps the matching correct if
// that ever changes and documents the intent.
function escapeRegExp( s ) {

	return s.replace( /[.*+?^${}()|[\]\\]/g, '\\$&' );

}

const WGSL_TYPE_ALIGN = {
	i32: 4, u32: 4, f32: 4, bool: 4,
	vec2f: 8, 'vec2<f32>': 8,
	vec3f: 16, 'vec3<f32>': 16,
	vec4f: 16, 'vec4<f32>': 16
};

const WGSL_TYPE_SIZE = {
	i32: 4, u32: 4, f32: 4, bool: 4,
	vec2f: 8, 'vec2<f32>': 8,
	vec3f: 12, 'vec3<f32>': 12,
	vec4f: 16, 'vec4<f32>': 16
};

/** WGSL uniform struct layout (WebGPU host-shareable rules). */
function layoutUniformStruct( fields ) {

	let offset = 0;
	let maxAlign = 4;
	const placements = [];
	for ( const field of fields ) {

		const align = WGSL_TYPE_ALIGN[ field.type ] || 4;
		const size = WGSL_TYPE_SIZE[ field.type ] || 4;
		offset = Math.ceil( offset / align ) * align;
		placements.push( { name: field.name, type: field.type, offset } );
		offset += size;
		maxAlign = Math.max( maxAlign, align );

	}
	const structSize = Math.ceil( offset / maxAlign ) * maxAlign;
	return { placements, structSize, vec4Stride: structSize / 16 };

}

/** Parse `array<StructName, N>` from a manifest uniform type string. */
function parseStructArrayType( typeStr, wgslSource, numLights, lightData, layout = null ) {

	const type = typeStr || '';
	const flatVec4 = type.match( /array<vec4f,\s*(\d+)>/ );
	if ( flatVec4 ) {

		const flatCount = parseInt( flatVec4[ 1 ], 10 );
		const stride = layout?.vec4Stride || 1;
		return {
			structName: 'LightData',
			count: Math.ceil( flatCount / stride ),
			vec4Stride: stride,
			flatCount
		};

	}
	const numeric = type.match( /array<(\w+),\s*(\d+)>/ );
	if ( numeric ) {

		const count = parseInt( numeric[ 2 ], 10 );
		return {
			structName: numeric[ 1 ],
			count,
			vec4Stride: layout?.vec4Stride || 1,
			flatCount: layout ? count * layout.vec4Stride : count
		};

	}
	const symbolic = type.match( /array<(\w+),\s*(\w+)>/ );
	const structName = symbolic ? symbolic[ 1 ] : 'LightData';
	let count = Math.max( numLights || 0, lightData?.length || 0, 1 );
	if ( symbolic && wgslSource ) {

		const constRe = new RegExp( `const\\s+${ escapeRegExp( symbolic[ 2 ] ) }:\\s*i32\\s*=\\s*(\\d+)` );
		const m = wgslSource.match( constRe );
		if ( m ) count = parseInt( m[ 1 ], 10 );

	}
	const vec4Stride = layout?.vec4Stride || 1;
	return { structName, count, vec4Stride, flatCount: count * vec4Stride };

}

/** Parse `struct LightData { ... }` from generated WGSL. */
export function parseLightDataStruct( wgsl ) {

	const m = wgsl.match( /struct\s+LightData\s*\{([^}]+)\}/ );
	if ( ! m ) return null;
	const fields = [];
	for ( const line of m[ 1 ].split( '\n' ) ) {

		const trimmed = line.trim().replace( /,$/, '' );
		if ( ! trimmed ) continue;
		const colon = trimmed.indexOf( ':' );
		if ( colon < 0 ) continue;
		fields.push( {
			name: trimmed.slice( 0, colon ).trim(),
			type: trimmed.slice( colon + 1 ).trim()
		} );

	}
	return fields.length ? layoutUniformStruct( fields ) : null;

}

/**
 * TSL `uniformArray` only binds `array<vec4*, N>`. Flatten `array<LightData, N>` to
 * `array<vec4f, N*stride>` and add an unpack helper for indexed struct access.
 */
function flattenLightDataUniform( wgsl ) {

	const layout = parseLightDataStruct( wgsl );
	if ( ! layout ) return { wgsl, layout: null };

	const maxMatch = wgsl.match( /array<LightData,\s*(\w+)\s*>/ );
	let maxCount = 1;
	if ( maxMatch ) {

		const sym = maxMatch[ 1 ];
		const constM = wgsl.match( new RegExp( `const\\s+${ escapeRegExp( sym ) }:\\s*i32\\s*=\\s*(\\d+)` ) );
		maxCount = constM ? parseInt( constM[ 1 ], 10 ) : 1;

	}
	const flatCount = maxCount * layout.vec4Stride;
	const stride = layout.vec4Stride;

	const unpackLines = [ 'var out: LightData;' ];
	for ( const p of layout.placements ) {

		const vec4Index = Math.floor( p.offset / 16 );
		const comp = [ 'x', 'y', 'z', 'w' ][ ( p.offset % 16 ) / 4 ];
		if ( p.type === 'vec3f' || p.type === 'vec3<f32>' ) {

			unpackLines.push( `out.${ p.name } = data[base + ${ vec4Index }u].xyz;` );

		} else if ( p.type === 'i32' ) {

			unpackLines.push( `out.${ p.name } = bitcast<i32>(data[base + ${ vec4Index }u].${ comp });` );

		} else if ( p.type === 'f32' ) {

			unpackLines.push( `out.${ p.name } = data[base + ${ vec4Index }u].${ comp };` );

		}

	}
	const helper = [
		`fn mx_lightData_at(data: array<vec4f, ${ flatCount }>, index: i32) -> LightData {`,
		`\tlet base: u32 = u32(index) * ${ stride }u;`,
		...unpackLines.map( ( l ) => `\t${ l }` ),
		'\treturn out;',
		'}'
	].join( '\n' );

	let out = wgsl.replace( /struct\s+LightData\s*\{/, `${ helper }\n\nstruct LightData {` );
	out = out.replace( /array<LightData,\s*[^>]+>/g, `array<vec4f, ${ flatCount }>` );
	out = out.replace( /u_lightData\[([^\]]+)\]/g, 'mx_lightData_at(u_lightData, $1)' );
	return { wgsl: out, layout };

}

function vec3ToArray( v ) {

	if ( Array.isArray( v ) ) return v;
	if ( v && v.x != null ) return [ v.x, v.y, v.z ];
	return [ 0, 0, 0 ];

}

// Reused buffer for packing i32 fields into vec4f slots (WGSL unpack uses bitcast<i32>).
const _intBitsView = new DataView( new ArrayBuffer( 4 ) );

/** Store an i32 in a float32 uniform slot preserving bit pattern for WGSL bitcast<i32>. */
function floatFromIntBits( i32 ) {

	_intBitsView.setInt32( 0, i32 | 0, true );
	return _intBitsView.getFloat32( 0, true );

}

/**
 * Pack light slots into a flat vec4 array matching WGSL uniform struct layout.
 * Inactive slots are zeroed so the WGSL light loop can skip them via u_numActiveLightSources.
 */
export function packLightDataToVec4Array( lightData, maxCount, layout ) {

	const vec4Count = maxCount * layout.vec4Stride;
	const floats = new Array( vec4Count * 4 ).fill( 0 );
	const floatStride = layout.vec4Stride * 4;
	for ( let i = 0; i < maxCount; i ++ ) {

		const l = lightData && lightData[ i ];
		const base = i * floatStride;
		if ( ! l ) continue;
		for ( const p of layout.placements ) {

			const floatIndex = base + p.offset / 4;
			if ( p.type === 'i32' || p.type === 'u32' ) {

				let intVal = 0;
				if ( p.name === 'light_type' || p.name === 'type' ) {

					intVal = l.light_type != null ? l.light_type : ( l.type | 0 );

				} else if ( l[ p.name ] != null ) {

					intVal = l[ p.name ] | 0;

				}
				floats[ floatIndex ] = floatFromIntBits( intVal );

			} else if ( p.name === 'direction' ) {

				const d = vec3ToArray( l.direction );
				floats[ floatIndex ] = d[ 0 ];
				floats[ floatIndex + 1 ] = d[ 1 ];
				floats[ floatIndex + 2 ] = d[ 2 ];

			} else if ( p.name === 'color' ) {

				const c = vec3ToArray( l.color );
				floats[ floatIndex ] = c[ 0 ];
				floats[ floatIndex + 1 ] = c[ 1 ];
				floats[ floatIndex + 2 ] = c[ 2 ];

			} else if ( p.name === 'intensity' ) {

				floats[ floatIndex ] = l.intensity != null ? l.intensity : 0;

			}

		}

	}
	const out = [];
	for ( let i = 0; i < vec4Count; i ++ ) {

		const b = i * 4;
		out.push( { x: floats[ b ], y: floats[ b + 1 ], z: floats[ b + 2 ], w: floats[ b + 3 ] } );

	}
	return out;

}

/**
 * Extract a top-level `fn <name>( <params> ) -> <ret> { <body> }` by name.
 * Returns { start, end, params, ret, body } where [start,end) spans the whole
 * definition in `src`. Throws if not found.
 */
function extractFunction( src, name ) {

	const re = new RegExp( `\\bfn\\s+${ escapeRegExp( name ) }\\s*\\(`, 'g' );
	const m = re.exec( src );
	if ( m === null ) throw new Error( `mxtsladapter: entry function '${ name }' not found` );

	const start = m.index;
	let i = m.index + m[ 0 ].length - 1; // at '('

	// Match the parameter-list parentheses.
	let depth = 0;
	const paramStart = i + 1;
	let paramEnd = - 1;
	for ( ; i < src.length; i ++ ) {

		if ( src[ i ] === '(' ) depth ++;
		else if ( src[ i ] === ')' ) { depth --; if ( depth === 0 ) { paramEnd = i; break; } }

	}

	if ( paramEnd < 0 ) throw new Error( `mxtsladapter: unterminated parameter list for '${ name }'` );

	const params = src.slice( paramStart, paramEnd );

	// Optional `-> type` then the body braces.
	let j = skipWs( src, paramEnd + 1 );
	let ret = '';
	if ( src[ j ] === '-' && src[ j + 1 ] === '>' ) {

		j = skipWs( src, j + 2 );
		const retStart = j;
		while ( j < src.length && src[ j ] !== '{' ) j ++;
		ret = src.slice( retStart, j ).trim();

	}

	if ( src[ j ] !== '{' ) throw new Error( `mxtsladapter: missing body for '${ name }'` );

	const bodyStart = j;
	depth = 0;
	let bodyEnd = - 1;
	for ( ; j < src.length; j ++ ) {

		if ( src[ j ] === '{' ) depth ++;
		else if ( src[ j ] === '}' ) { depth --; if ( depth === 0 ) { bodyEnd = j + 1; break; } }

	}

	if ( bodyEnd < 0 ) throw new Error( `mxtsladapter: unterminated body for '${ name }'` );

	return {
		start,
		end: bodyEnd,
		params: params.trim(),
		ret,
		body: src.slice( bodyStart + 1, bodyEnd - 1 ) // inside the outer braces
	};

}

/** Remove every line that declares a `@group(...) @binding(...)` resource. */
function removeBindingDecls( src ) {

	return src
		.split( '\n' )
		.filter( ( line ) => ! /^\s*@group\s*\(/.test( line ) )
		.join( '\n' );

}

/** Index of the `)` matching the `(` at `openIdx` in `src`. */
function matchParen( src, openIdx ) {

	let depth = 0;
	for ( let i = openIdx; i < src.length; i ++ ) {

		if ( src[ i ] === '(' ) depth ++;
		else if ( src[ i ] === ')' ) { depth --; if ( depth === 0 ) return i; }

	}
	return - 1;

}

/**
 * Thread private/global HW resources through the call chain.
 *
 * The MaterialX library functions (IBL, transmission, lighting) reference the HW private
 * uniforms + env maps as *module-scope globals* (`u_envMatrix`, `u_viewPosition`,
 * `u_lightData`, the env textures, …). Three.js `wgslFn` forbids module-scope resources in
 * `includes`, so any function that (transitively) touches one of them must receive it as a
 * parameter, and every call site must forward it.
 *
 * Only these private/global resources are threaded. Public material inputs (semantic
 * `uniform`) and material image textures (semantic `texture`/`sampler`) must NOT be threaded:
 * they are part of the MaterialX node interface and are already passed explicitly down the
 * node-graph call chain as named parameters (e.g. `NG_..._surfaceshader_100(base, base_color, …)`).
 * Threading them would re-append them to functions that already declare them as parameters or
 * use them as local variable names, producing WGSL "redeclaration of '…'" errors.
 *
 * The entry function is left unmodified — `convertToTslPortable` adds the resources to it as
 * parameters from the manifest, and call sites in the entry body forward them.
 *
 * @param {string} wgsl - The full generated WGSL module.
 * @param {Object} manifest - Generator manifest (uniforms + textures after normalization).
 * @return {string} The rewritten module (unchanged if there are no private resources).
 */
export function threadEnvResources( wgsl, manifest ) {

	// The generator references geometry varyings through the MaterialX vertex-data instance
	// `vd` (e.g. `vd.normalWorld`). The entry exposes the `VertexData` members as flat
	// parameters (and the assembler binds them to TSL builtins), so flatten the struct access
	// to bare member names module-wide. The varyings are then threaded like any other global
	// below, so library helpers (e.g. the surface-shader node) that read them get them as
	// parameters forwarded from the entry. `\bvd\.` only matches the vertex-data instance.
	wgsl = wgsl.replace( /\bvd\./g, '' );

	// The native WgslShaderGenerator threads the whole `vd: VertexData` struct into closure /
	// surface-shader functions as an explicit parameter (and forwards `vd` at each call site),
	// because standalone WGSL has no module-scope varyings. The TSL bridge instead threads the
	// individual varyings (flattened to bare names above and re-added as params below), so the
	// struct parameter and its forwarded argument are redundant — strip them. The entry's sole
	// `vd: VertexData` parameter is intentionally left intact so `convertToTslPortable` can still
	// detect the struct entry and rebuild it from the manifest's flat varying members.
	wgsl = wgsl.replace( /\bvd\s*:\s*VertexData\s*,\s*/g, '' ); // first parameter of a helper
	wgsl = wgsl.replace( /,\s*vd\s*:\s*VertexData\b/g, '' );    // later parameter of a helper
	wgsl = wgsl.replace( /\(\s*vd\s*,\s*/g, '(' );              // forwarded `vd` as first argument
	wgsl = wgsl.replace( /\(\s*vd\s*\)/g, '()' );               // forwarded `vd` as sole argument

	// A resource is "private/global" (referenced as a module-scope global inside library
	// helpers, hence threaded) when it is an env map or a non-public uniform. Public material
	// inputs (`uniform`) and material image textures (`texture`/`sampler`) flow through the
	// node-graph parameter chain and are excluded.
	const isPrivateTexture = ( t ) => ( t.semantic || '' ).startsWith( 'env:' );
	const isPrivateUniform = ( u ) => ( u.semantic || 'uniform' ) !== 'uniform';

	// Ordered resource list (signature decls + call args), data-driven from the manifest.
	const resourceParts = [];

	// Vertex-data varyings (now bare member names after the `vd.` flatten above). These are
	// genuine module inputs that helpers reference globally, so they must be threaded too.
	for ( const p of manifest.entryParams || [] ) {

		if ( ! ( p.semantic || '' ).startsWith( 'varying:' ) ) continue;
		resourceParts.push( { decl: `${ p.name }: ${ p.type }`, arg: p.name } );

	}
	for ( const t of manifest.textures || [] ) {

		if ( ! isPrivateTexture( t ) ) continue;
		resourceParts.push( { decl: `${ t.texture }: ${ t.wgslType }`, arg: t.texture } );
		resourceParts.push( { decl: `${ t.sampler }: sampler`, arg: t.sampler } );

	}
	for ( const u of manifest.uniforms || [] ) {

		if ( ! isPrivateUniform( u ) ) continue;
		resourceParts.push( { decl: `${ u.name }: ${ u.type }`, arg: u.name } );

	}
	if ( resourceParts.length === 0 ) return wgsl;

	const resourceNames = resourceParts.map( ( p ) => p.arg );
	const paramsStr = resourceParts.map( ( p ) => p.decl ).join( ', ' );
	const argsStr = resourceParts.map( ( p ) => p.arg ).join( ', ' );

	// Enumerate all top-level `fn name( params ) ... { body }` definitions.
	const fnRe = /\bfn\s+([A-Za-z_]\w*)\s*\(/g;
	const fns = [];
	for ( let m = fnRe.exec( wgsl ); m !== null; m = fnRe.exec( wgsl ) ) {

		const openParen = m.index + m[ 0 ].length - 1;
		const closeParen = matchParen( wgsl, openParen );
		const braceOpen = wgsl.indexOf( '{', closeParen );
		// Match the body braces to know where this function ends.
		let depth = 0, braceEnd = - 1;
		for ( let i = braceOpen; i < wgsl.length; i ++ ) {

			if ( wgsl[ i ] === '{' ) depth ++;
			else if ( wgsl[ i ] === '}' ) { depth --; if ( depth === 0 ) { braceEnd = i; break; } }

		}
		fns.push( { name: m[ 1 ], openParen, closeParen, bodyStart: braceOpen, bodyEnd: braceEnd } );

	}

	const byName = Object.fromEntries( fns.map( ( f ) => [ f.name, f ] ) );
	const bodyOf = ( f ) => wgsl.slice( f.bodyStart, f.bodyEnd + 1 );

	// Fixpoint: a function "needs" resources if its body references one directly, or
	// calls another function that needs them.
	const needs = new Set();
	const refsResource = ( body ) => resourceNames.some( ( n ) => new RegExp( `\\b${ escapeRegExp( n ) }\\b` ).test( body ) );
	for ( const f of fns ) if ( refsResource( bodyOf( f ) ) ) needs.add( f.name );

	for ( let changed = true; changed; ) {

		changed = false;
		for ( const f of fns ) {

			if ( needs.has( f.name ) ) continue;
			const body = bodyOf( f );
			for ( const callee of needs ) {

				if ( new RegExp( `\\b${ escapeRegExp( callee ) }\\s*\\(` ).test( body ) ) { needs.add( f.name ); changed = true; break; }

			}

		}

	}

	if ( needs.size === 0 ) return wgsl;

	const entryName = manifest.entry;

	// Collect insertions (index -> text). Apply right-to-left so indices stay valid.
	const inserts = [];

	// 1. Append params to the signature of each needing function (except the entry).
	for ( const f of fns ) {

		if ( ! needs.has( f.name ) || f.name === entryName ) continue;
		const hasParams = wgsl.slice( f.openParen + 1, f.closeParen ).trim().length > 0;
		inserts.push( { at: f.closeParen, text: ( hasParams ? ', ' : '' ) + paramsStr } );

	}

	// 2. Append args at every call site of a needing function (anywhere in the module).
	for ( const callee of needs ) {

		const callRe = new RegExp( `\\b${ escapeRegExp( callee ) }\\s*\\(`, 'g' );
		for ( let m = callRe.exec( wgsl ); m !== null; m = callRe.exec( wgsl ) ) {

			// Skip the definition itself (`fn callee(`).
			const before = wgsl.slice( Math.max( 0, m.index - 4 ), m.index );
			if ( /\bfn\s$/.test( before ) ) continue;
			const open = m.index + m[ 0 ].length - 1;
			const close = matchParen( wgsl, open );
			if ( close < 0 ) continue;
			const hasArgs = wgsl.slice( open + 1, close ).trim().length > 0;
			inserts.push( { at: close, text: ( hasArgs ? ', ' : '' ) + argsStr } );

		}

	}

	inserts.sort( ( a, b ) => b.at - a.at );
	let out = wgsl;
	for ( const ins of inserts ) out = out.slice( 0, ins.at ) + ins.text + out.slice( ins.at );
	return out;

}

// ── Reflection normalization (neutral C++ → TSL semantics) ─────────────────

function varyingSemantic( name ) {

	if ( name.includes( 'normal' ) ) return 'varying:normalWorld';
	if ( name.includes( 'tangent' ) ) return 'varying:tangentWorld';
	if ( name.includes( 'position' ) ) return 'varying:positionWorld';
	return 'varying:uv';

}

function buildTslSemantics( binding ) {

	const { name, role } = binding;
	const HOST = {
		u_viewPosition: 'camera:viewPosition',
		u_numActiveLightSources: 'light:count',
		u_lightData: 'light:data',
		u_envMatrix: 'env:matrix',
		u_envRadianceMips: 'env:radianceMips',
		u_envRadianceSamples: 'env:radianceSamples',
		u_envLightIntensity: 'env:lightIntensity'
	};
	if ( HOST[ name ] ) return HOST[ name ];
	if ( name === 'u_lightDirection' ) return 'light:direction';
	if ( name === 'u_lightColor' ) return 'light:color';
	if ( role === 'lightData' ) return 'light:data';
	if ( role === 'uniform' ) return 'uniform';
	if ( role === 'texture' ) {

		if ( name.startsWith( 'u_envRadiance' ) ) return 'env:radiance';
		if ( name.startsWith( 'u_envIrradiance' ) ) return 'env:irradiance';
		return 'texture';

	}
	if ( role === 'sampler' ) {

		if ( name.startsWith( 'u_envRadiance' ) ) return 'env:radiance:sampler';
		if ( name.startsWith( 'u_envIrradiance' ) ) return 'env:irradiance:sampler';
		return 'sampler';

	}
	if ( role === 'host' ) return HOST[ name ] || 'host';
	return 'uniform';

}

/**
 * Normalize generator reflection into the legacy manifest shape expected by the
 * TSL bridge. Accepts both the old manifest (entry string + uniforms/textures)
 * and the new neutral format (entry object + bindings + struct entryParams).
 *
 * @param {Object} reflection - Generator reflection or legacy manifest.
 * @return {Object} Manifest with TSL semantics on uniforms, textures, entryParams.
 */
export function normalizeReflection( reflection ) {

	if ( ! reflection ) return reflection;

	// Legacy manifests already carry TSL semantics — return unchanged.
	if ( reflection.uniforms?.some( ( u ) => u.semantic ) || reflection.textures?.some( ( t ) => t.semantic ) )
		return reflection;

	if ( ! reflection.bindings ) return reflection;

	const entry = typeof reflection.entry === 'string'
		? reflection.entry
		: reflection.entry?.pixel;

	const uniforms = [];
	const textures = [];
	const textureKeys = new Set();

	for ( const b of reflection.bindings ) {

		const semantic = buildTslSemantics( b );
		if ( b.role === 'texture' ) {

			const key = b.key || b.name.replace( /_texture$/, '' );
			textures.push( {
				key,
				texture: b.name,
				sampler: null,
				wgslType: b.type,
				semantic,
				file: b.file
			} );
			textureKeys.add( key );

		} else if ( b.role === 'sampler' ) {

			const key = b.key || b.name.replace( /_sampler$/, '' );
			const tex = textures.find( ( t ) => t.key === key );
			if ( tex ) tex.sampler = b.name;

		} else {

			uniforms.push( {
				name: b.name,
				type: b.type,
				semantic,
				value: b.value
			} );

		}

	}

	for ( const t of textures ) {

		if ( ! t.sampler ) t.sampler = `${ t.key }_sampler`;

	}

	let entryParams = reflection.entryParams || [];
	if ( entryParams.length === 1 && entryParams[ 0 ].members ) {

		entryParams = entryParams[ 0 ].members.map( ( m ) => ( {
			name: m.name,
			type: m.type,
			semantic: varyingSemantic( m.name )
		} ) );

	} else {

		entryParams = entryParams.map( ( p ) => ( {
			...p,
			semantic: p.semantic || varyingSemantic( p.name )
		} ) );

	}

	return {
		entry,
		output: reflection.output,
		entryParams,
		uniforms,
		textures
	};

}

// ── Conversion ──────────────────────────────────────────────────────────────

/**
 * Convert a complete MaterialX WGSL module into a TSL-portable form.
 *
 * @param {string} wgsl - The generated WGSL module.
 * @param {Object} manifest - Resource description emitted by the generator.
 * @return {{ name:string, entry:string, includes:string, params:Array }}
 */
export function convertToTslPortable( wgsl, manifest ) {

	manifest = normalizeReflection( manifest );

	// Thread module-scope resources through the call chain so `includes` receive them
	// as parameters rather than globals (required by Three.js `wgslFn`).
	wgsl = threadEnvResources( wgsl, manifest );

	// Three.js uniformArray only supports primitive array types (vec4/mat4), not struct arrays.
	const lightFlatten = flattenLightDataUniform( wgsl );
	wgsl = lightFlatten.wgsl;
	const lightLayout = lightFlatten.layout;

	const entryName = manifest.entry;
	const fn = extractFunction( wgsl, entryName );

	const uniforms = manifest.uniforms || [];
	const textures = manifest.textures || [];
	const entryParams = manifest.entryParams || [];

	// The generated entry already takes the geometry varyings as parameters and
	// references uniforms/textures by bare name as module-scope `@group/@binding`
	// resources. Hoist those resources into the parameter list (bodies use bare
	// names, so no in-body rewriting is needed) and strip the binding declarations.
	const newParams = [];
	const params = [];

	// 1. Existing entry parameters (varyings). Neutral reflection may pass vertex
	// data as a single struct parameter — flatten it for wgslFn and rewrite the body.
	const semanticByName = Object.fromEntries( entryParams.map( ( p ) => [ p.name, p.semantic ] ) );
	const flatParams = splitParams( fn.params );
	const isStructEntry = flatParams.length === 1 && entryParams.length > 0 &&
		entryParams[ 0 ].name !== flatParams[ 0 ].name;
	let body = fn.body;
	if ( isStructEntry ) {

		const structName = flatParams[ 0 ].name;
		for ( const m of entryParams ) {

			newParams.push( `${ m.name }: ${ m.type }` );
			params.push( { name: m.name, type: m.type, semantic: m.semantic || varyingSemantic( m.name ) } );

		}
		body = body.replace( new RegExp( `\\b${ escapeRegExp( structName ) }\\.`, 'g' ), '' );

	} else {

		for ( const p of flatParams ) {

			newParams.push( `${ p.name }: ${ p.type }` );
			params.push( { name: p.name, type: p.type, semantic: semanticByName[ p.name ] || 'varying:uv' } );

		}

	}

	// 2. Uniforms become individual parameters (bare names already used in body).
	//    Most carry semantic 'uniform'; surface lighting inputs carry camera:/light:.
	for ( const u of uniforms ) {

		let paramType = u.type;
		if ( ( u.semantic || '' ) === 'light:data' && lightLayout ) {

			const { flatCount } = parseStructArrayType( u.type, wgsl, null, null, lightLayout );
			paramType = `array<vec4f, ${ flatCount }>`;

		}
		newParams.push( `${ u.name }: ${ paramType }` );
		params.push( { name: u.name, type: paramType, semantic: u.semantic || 'uniform', value: u.value } );

	}

	// 3. Each texture contributes a texture + sampler parameter pair. Material image
	//    textures carry semantic 'texture'; env (IBL) maps carry 'env:radiance' /
	//    'env:irradiance' so the assembler can bind them from the environment instead.
	for ( const tex of textures ) {

		const texSemantic = tex.semantic && tex.semantic.startsWith( 'env:' ) ? tex.semantic : 'texture';
		const sampSemantic = texSemantic === 'texture' ? 'sampler' : texSemantic + ':sampler';
		newParams.push( `${ tex.texture }: ${ tex.wgslType }` );
		newParams.push( `${ tex.sampler }: sampler` );
		params.push( { name: tex.texture, type: tex.wgslType, semantic: texSemantic, key: tex.key } );
		params.push( { name: tex.sampler, type: 'sampler', semantic: sampSemantic, key: tex.key } );

	}

	// Strip entry-point IO attributes (e.g. `@location(0)`, `@builtin(...)`) from the return
	// type: they are required on the standalone `@fragment` entry (WGSL validation) but invalid
	// on the plain function that Three.js `wgslFn` turns this into.
	const retType = fn.ret ? fn.ret.replace( /@\w+\s*\([^)]*\)\s*/g, '' ).trim() : '';
	const retArrow = retType ? ` -> ${ retType }` : '';
	const entry = `fn ${ entryName }( ${ newParams.join( ', ' ) } )${ retArrow } {${ body }}`;

	// Includes: the original module minus the entry and the binding declarations.
	let includes = wgsl.slice( 0, fn.start ) + wgsl.slice( fn.end );
	includes = removeBindingDecls( includes ).trim();

	// The generator emits the stage attribute on the line *before* `fn <entry>` (e.g.
	// `@fragment\nfn fragmentMain(...)`) for standalone WGSL validation. `extractFunction`
	// starts at `fn`, so that attribute is left dangling in the includes and would re-attach
	// to the next function Three.js emits — turning a plain helper into a bogus entry point
	// ("missing entry point IO attribute on parameter"). Strip any stray stage attributes.
	includes = includes.replace( /@(?:fragment|vertex|compute)\b\s*/g, '' ).trim();

	// The vertex-data struct is now unused: the entry takes flat varying parameters and the
	// helper functions were de-threaded (see threadEnvResources). It also carries @builtin /
	// @location IO attributes that are only valid at an entry boundary, so leaving its
	// definition in the wgslFn includes would be rejected. Strip it.
	if ( isStructEntry ) {

		const structType = flatParams[ 0 ].type;
		includes = includes
			.replace( new RegExp( `struct\\s+${ escapeRegExp( structType ) }\\s*\\{[^}]*\\}\\s*`, 'g' ), '' )
			.trim();

	}

	return { name: entryName, entry, includes, params };

}

/** Split a WGSL parameter list ("a: T1, b: T2") into [{name,type}]. */
function splitParams( params ) {

	const out = [];
	let depth = 0, start = 0;
	const pieces = [];
	for ( let i = 0; i < params.length; i ++ ) {

		const c = params[ i ];
		if ( c === '<' || c === '(' ) depth ++;
		else if ( c === '>' || c === ')' ) depth --;
		else if ( c === ',' && depth === 0 ) { pieces.push( params.slice( start, i ) ); start = i + 1; }

	}
	const tail = params.slice( start ).trim();
	if ( tail ) pieces.push( tail );

	for ( const piece of pieces ) {

		const colon = piece.indexOf( ':' );
		if ( colon < 0 ) continue;
		out.push( { name: piece.slice( 0, colon ).trim(), type: piece.slice( colon + 1 ).trim() } );

	}
	return out;

}

// ── Material assembly ────────────────────────────────────────────────────────

/**
 * Build a Three.js NodeMaterial from MaterialX WGSL + manifest.
 *
 * @param {Object} options
 * @param {Object} options.THREE - The `three/webgpu` namespace.
 * @param {Object} options.TSL - The `three/tsl` namespace.
 * @param {string} options.wgsl - Generated WGSL module.
 * @param {Object} options.manifest - Generator manifest.
 * @param {Object<string,Texture>} [options.textures] - Map of texture key -> Texture.
 * @param {Node} [options.uvNode] - Optional uv node (defaults to TSL `uv()`).
 * @param {boolean} [options.useGeometryTangent] - Use TSL `tangentWorld` when geometry tangents exist.
 * @return {NodeMaterial}
 */
export function createMxWgslMaterial( { THREE, TSL, wgsl, manifest, textures = {}, uvNode = null, light = null, lightData = null, numLights = null, environment = null, useGeometryTangent = false } ) {

	manifest = normalizeReflection( manifest );

	const { wgslFn, wgsl: wgslCode, uniform, uniformArray, texture, sampler, uv, normalWorld, positionWorld, cameraPosition, tangentWorld: tangentWorldBuiltin, vec3, float, select } = TSL;

	// IBL environment: equirect radiance (mipped) + irradiance maps and FIS parameters.
	// When the shader was generated with FIS but no environment is supplied, bind a 1×1
	// black fallback so the module still compiles (IBL simply contributes nothing).
	const env = environment || {};
	const fallbackEnvTex = () => {

		const t = new THREE.DataTexture( new Uint8Array( [ 0, 0, 0, 255 ] ), 1, 1 );
		t.needsUpdate = true;
		return t;

	};
	const envRadianceTex = env.radiance || fallbackEnvTex();
	const envIrradianceTex = env.irradiance || envRadianceTex;
	// HDR env maps must stay linear (see viewer buildEnvironment).
	envRadianceTex.colorSpace = THREE.NoColorSpace;
	envIrradianceTex.colorSpace = THREE.NoColorSpace;
	const envTexFor = ( semantic ) => semantic.startsWith( 'env:irradiance' ) ? envIrradianceTex : envRadianceTex;
	const fallbackMatTex = () => {

		const t = new THREE.DataTexture( new Uint8Array( [ 128, 128, 128, 255 ] ), 1, 1 );
		// DataTexture defaults to Nearest+Nearest; Three.js treats that as "unfilterable" and
		// omits nodeUniformN_sampler bindings while MaterialX WGSL still passes samplers.
		t.magFilter = THREE.LinearFilter;
		t.minFilter = THREE.LinearFilter;
		t.needsUpdate = true;
		return t;

	};

	// Three.js only emits sampler bindings for filterable textures (see WebGPUNodeBuilder
	// needsSampler). MaterialX WGSL always declares matching sampler parameters.
	const ensureFilterableTexture = ( tex ) => {

		if ( ! tex ) return tex;
		if ( tex.magFilter === THREE.NearestFilter ) tex.magFilter = THREE.LinearFilter;
		if ( tex.minFilter === THREE.NearestFilter || tex.minFilter === THREE.NearestMipmapNearestFilter ) {

			tex.minFilter = tex.generateMipmaps !== false ? THREE.LinearMipmapLinearFilter : THREE.LinearFilter;

		}
		return tex;

	};

	// A robust world-space tangent. MaterialX BSDFs orthogonalize the tangent against
	// the normal (`normalize(T - dot(T,N)*N)`), which yields NaN if T is zero or parallel
	// to N — and most geometries (e.g. TorusKnotGeometry) carry no tangent attribute, so
	// a raw `tangentWorld` accessor would be degenerate. Derive an arbitrary orthonormal
	// tangent from the normal instead; for isotropic GGX the exact direction is irrelevant.
	function derivedTangentWorld() {

		const n = normalWorld.normalize();
		// Reference axis least aligned with the normal, to avoid a parallel cross product.
		const ref = select( n.y.abs().lessThan( float( 0.99 ) ), vec3( 0, 1, 0 ), vec3( 1, 0, 0 ) );
		return ref.cross( n ).normalize();

	}
	// MaterialXView computes tangents for indexed geometry; use them when available for
	// anisotropic parity with the WebGL path. Fall back to a derived tangent otherwise.
	const tangentWorld = useGeometryTangent ? tangentWorldBuiltin : derivedTangentWorld();

	const converted = convertToTslPortable( wgsl, manifest );
	const wgslForConsts = converted.includes + '\n' + converted.entry;

	const includesNode = converted.includes.length ? [ wgslCode( converted.includes ) ] : [];
	const entryFn = wgslFn( converted.entry, includesNode );

	const builtin = {
		'varying:uv': () => ( uvNode || uv() ),
		'varying:normalWorld': () => normalWorld,
		'varying:positionWorld': () => positionWorld,
		'varying:tangentWorld': () => tangentWorld,
		'camera:viewPosition': () => cameraPosition
	};

	const args = {};
	// One TSL texture() node per manifest texture key. wgslFn passes texture and sampler as
	// separate WGSL parameters; calling texture() twice (even with the same THREE.Texture)
	// creates two binding slots (nodeUniformN + nodeUniformN+1_sampler) and breaks pairing.
	const textureNodes = {};
	const textureNodeFor = ( key, threeTex ) => {

		if ( ! textureNodes[ key ] ) textureNodes[ key ] = texture( ensureFilterableTexture( threeTex ) );
		return textureNodes[ key ];

	};

	for ( const p of converted.params ) {

		if ( p.semantic === 'uniform' ) {

			args[ p.name ] = makeUniformNode( uniform, toUniformValue( THREE, p ), p.type );

		} else if ( p.semantic === 'light:direction' ) {

			args[ p.name ] = uniform( ( light && light.direction ) || toUniformValue( THREE, p ) );

		} else if ( p.semantic === 'light:color' ) {

			args[ p.name ] = uniform( ( light && light.color ) || toUniformValue( THREE, p ) );

		} else if ( p.semantic === 'light:count' ) {

			args[ p.name ] = uniform( ( numLights != null ? numLights : p.value ) | 0, 'int' );

		} else if ( p.semantic === 'light:data' ) {

			const layout = parseLightDataStruct( wgslForConsts );
			const { count, flatCount } = parseStructArrayType( p.type, wgslForConsts, null, lightData, layout );
			const packed = layout
				? packLightDataToVec4Array( lightData || p.value, count, layout )
				: new Array( flatCount ).fill( { x: 0, y: 0, z: 0, w: 0 } );
			args[ p.name ] = uniformArray( packed, 'vec4' );

		} else if ( p.semantic === 'host' ) {

			args[ p.name ] = makeUniformNode( uniform, toUniformValue( THREE, p ), p.type );

		} else if ( p.semantic === 'texture' ) {

			args[ p.name ] = textureNodeFor( p.key, textures[ p.key ] || fallbackMatTex() );

		} else if ( p.semantic === 'sampler' ) {

			args[ p.name ] = sampler( textureNodeFor( p.key, textures[ p.key ] || fallbackMatTex() ) );

		} else if ( p.semantic === 'env:radiance' || p.semantic === 'env:irradiance' ) {

			args[ p.name ] = textureNodeFor( p.key, envTexFor( p.semantic ) );

		} else if ( p.semantic === 'env:radiance:sampler' || p.semantic === 'env:irradiance:sampler' ) {

			const envSemantic = p.semantic.replace( ':sampler', '' );
			args[ p.name ] = sampler( textureNodeFor( p.key, envTexFor( envSemantic ) ) );

		} else if ( p.semantic === 'env:matrix' ) {

			args[ p.name ] = uniform( env.matrix || matrixFromArray( THREE, p.value ) );

		} else if ( p.semantic === 'env:radianceSamples' ) {

			args[ p.name ] = uniform( ( env.samples != null ? env.samples : p.value ) | 0, 'int' );

		} else if ( p.semantic === 'env:radianceMips' ) {

			args[ p.name ] = uniform( ( env.mips != null ? env.mips : p.value ) | 0, 'int' );

		} else if ( p.semantic === 'env:lightIntensity' ) {

			args[ p.name ] = uniform( env.intensity != null ? env.intensity : p.value );

		} else if ( p.semantic && ( p.semantic.startsWith( 'varying:' ) || p.semantic.startsWith( 'camera:' ) ) ) {

			const make = builtin[ p.semantic ];
			if ( ! make ) throw new Error( `mxtsladapter: unknown builtin semantic '${ p.semantic }'` );
			args[ p.name ] = make();

		} else {

			throw new Error( `mxtsladapter: unhandled parameter semantic '${ p.semantic }' for '${ p.name }'` );

		}

	}

	const material = new THREE.MeshBasicNodeMaterial();
	// MaterialX WGSL already includes direct lights + IBL; do not run Three.js scene lighting.
	material.lights = false;
	material.fog = false;
	material.colorNode = entryFn( args );
	// Bound argument nodes, keyed by parameter name. `createMxWgslGUI` reads these to
	// drive live uniform edits, so this is a functional binding handle, not debug state.
	material.userData.mxArgs = args;
	return material;

}

function toUniformValue( THREE, p ) {

	const v = p.value;
	if ( v === undefined || v === null ) {

		// Sensible zero/identity defaults by type.
		if ( p.type === 'f32' || p.type === 'i32' || p.type === 'u32' ) return 0;
		if ( p.type === 'vec2f' ) return new THREE.Vector2();
		if ( p.type === 'vec4f' ) return new THREE.Vector4();
		return new THREE.Vector3();

	}

	if ( typeof v === 'boolean' && ( p.type === 'i32' || p.type === 'u32' ) ) return v ? 1 : 0;

	if ( Array.isArray( v ) ) {

		if ( v.length === 2 ) return new THREE.Vector2( v[ 0 ], v[ 1 ] );
		if ( v.length === 3 ) return new THREE.Vector3( v[ 0 ], v[ 1 ], v[ 2 ] );
		if ( v.length === 4 ) return new THREE.Vector4( v[ 0 ], v[ 1 ], v[ 2 ], v[ 3 ] );
		if ( v.length === 16 ) return matrixFromArray( THREE, v );

	}

	return v;

}

/** Bind a TSL uniform node with the correct scalar type for WGSL i32/u32 host inputs. */
function makeUniformNode( uniformFn, value, wgslType ) {

	if ( wgslType === 'i32' || wgslType === 'u32' ) return uniformFn( value | 0, 'int' );
	return uniformFn( value );

}

/** Build a THREE.Matrix4 from a 16-element column-major array (WGSL mat4x4f layout). */
function matrixFromArray( THREE, v ) {

	const m = new THREE.Matrix4();
	if ( Array.isArray( v ) && v.length === 16 ) {

		// Three stores column-major internally; WGSL mat4x4f is also column-major, so the
		// array maps directly into Matrix4.elements.
		m.elements = v.slice();

	}
	return m;

}

// ── Live parameter editor ─────────────────────────────────────────────────────

// Slider range heuristics for scalar surface inputs, keyed by name substring.
function scalarRange( name ) {

	if ( /IOR$/.test( name ) ) return [ 1, 3 ];
	if ( /thickness/.test( name ) ) return [ 0, 2000 ]; // thin-film, nanometres
	if ( /roughness|metalness|anisotropy|rotation|affect|walled/.test( name ) ) return [ 0, 1 ];
	if ( /emission|scale|depth|dispersion/.test( name ) ) return [ 0, 5 ];
	return [ 0, 1 ];

}

const GUI_CATEGORIES = [ 'base', 'diffuse', 'metalness', 'specular', 'transmission',
	'subsurface', 'sheen', 'coat', 'thin_film', 'thin_walled', 'emission', 'opacity' ];

function guiCategory( stripped ) {

	for ( const c of GUI_CATEGORIES ) if ( stripped === c || stripped.startsWith( c + '_' ) ) return c;
	return 'other';

}

/**
 * Build a live lil-gui editor for a material created by `createMxWgslMaterial`.
 *
 * Drives the bound TSL uniform nodes (`material.userData.mxArgs`) directly, so edits take
 * effect on the next render with no rebuild. Surface inputs are read from the manifest and
 * grouped into folders by closure (base / specular / coat / …); the directional light and
 * environment intensity get their own folder.
 *
 * @param {Object} options
 * @param {Function} options.GUI - The lil-gui constructor.
 * @param {Object} options.THREE - The `three/webgpu` namespace.
 * @param {NodeMaterial} options.material - Material from `createMxWgslMaterial`.
 * @param {Object} options.manifest - The generator manifest used to build the material.
 * @param {GUI} [options.gui] - Existing gui to populate (otherwise a new one is created).
 * @return {GUI}
 */
export function createMxWgslGUI( { GUI, THREE, material, manifest, gui = null } ) {

	manifest = normalizeReflection( manifest );
	gui = gui || new GUI();
	const args = material.userData.mxArgs || {};

	const folders = {};
	const folderFor = ( parent, key, label ) => {

		const id = parent + '/' + key;
		if ( ! folders[ id ] ) { folders[ id ] = ( parent === '' ? gui : folders[ parent ] ).addFolder( label ); folders[ id ].close(); }
		return folders[ id ];

	};

	// One control bound to a uniform node's `.value`.
	const addControl = ( folder, node, type, name, value, label, range ) => {

		if ( type === 'vec3f' && /color|scatter/.test( name ) ) {

			const proxy = { c: Array.isArray( value ) ? value.slice() : [ value.x, value.y, value.z ] };
			folder.addColor( proxy, 'c' ).name( label ).onChange( ( v ) => node.value.fromArray( v ) );

		} else if ( type === 'vec3f' ) {

			const sub = folder.addFolder( label ); sub.close();
			const proxy = Array.isArray( value ) ? { x: value[ 0 ], y: value[ 1 ], z: value[ 2 ] } : { x: value.x, y: value.y, z: value.z };
			const lo = range ? range[ 0 ] : 0, hi = range ? range[ 1 ] : 2;
			for ( const k of [ 'x', 'y', 'z' ] )
				sub.add( proxy, k, lo, hi ).onChange( () => node.value.set( proxy.x, proxy.y, proxy.z ) );

		} else if ( type === 'bool' ) {

			const proxy = { v: !! value };
			folder.add( proxy, 'v' ).name( label ).onChange( ( v ) => { node.value = v; } );

		} else if ( type === 'vec2f' ) {

			const sub = folder.addFolder( label ); sub.close();
			const proxy = Array.isArray( value ) ? { x: value[ 0 ], y: value[ 1 ] } : { x: value.x, y: value.y };
			const lo = range ? range[ 0 ] : 0, hi = range ? range[ 1 ] : 2;
			for ( const k of [ 'x', 'y' ] )
				sub.add( proxy, k, lo, hi ).onChange( () => node.value.set( proxy.x, proxy.y ) );

		} else if ( type === 'vec4f' ) {

			const sub = folder.addFolder( label ); sub.close();
			const proxy = Array.isArray( value )
				? { x: value[ 0 ], y: value[ 1 ], z: value[ 2 ], w: value[ 3 ] }
				: { x: value.x, y: value.y, z: value.z, w: value.w };
			const lo = range ? range[ 0 ] : 0, hi = range ? range[ 1 ] : 2;
			for ( const k of [ 'x', 'y', 'z', 'w' ] )
				sub.add( proxy, k, lo, hi ).onChange( () => node.value.set( proxy.x, proxy.y, proxy.z, proxy.w ) );

		} else if ( type === 'f32' || type === 'i32' || type === 'u32' ) {

			const [ lo, hi ] = range || scalarRange( name );
			const proxy = { v: Number( value ) || 0 };
			const ctrl = folder.add( proxy, 'v', lo, hi ).name( label ).onChange( ( v ) => { node.value = v; } );
			if ( type === 'i32' || type === 'u32' ) ctrl.step( 1 );

		}

	};

	// Surface inputs (semantic 'uniform'), grouped by closure category.
	const surface = folderFor( '', 'surface', 'MaterialX surface' );
	surface.open();
	for ( const u of manifest.uniforms || [] ) {

		if ( u.semantic !== 'uniform' ) continue;
		const node = args[ u.name ];
		if ( ! node || u.type === 'mat4x4f' ) continue;
		const stripped = u.name.replace( /^[A-Za-z][A-Za-z0-9]*_/, '' ); // drop node prefix (e.g. 'ss_')
		const cat = guiCategory( stripped );
		const folder = cat === 'other' ? surface : folderFor( '/surface', cat, cat.replace( /_/g, ' ' ) );
		addControl( folder, node, u.type, u.name, u.value, stripped, null );

	}

	// Lighting + environment.
	const lighting = folderFor( '', 'lighting', 'Lighting & environment' );
	for ( const u of manifest.uniforms || [] ) {

		const node = args[ u.name ];
		if ( ! node ) continue;
		if ( u.semantic === 'light:direction' ) addControl( lighting, node, 'vec3f', 'dir_vec', u.value, 'light direction', [ - 1, 1 ] );
		else if ( u.semantic === 'light:color' ) addControl( lighting, node, 'vec3f', 'light_rgb', u.value, 'light color', [ 0, 10 ] );
		else if ( u.semantic === 'env:lightIntensity' ) addControl( lighting, node, 'f32', 'env_intensity', u.value, 'env intensity', [ 0, 3 ] );

	}

	return gui;

}
