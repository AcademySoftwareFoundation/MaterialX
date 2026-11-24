#!/usr/bin/env node
/**
 * Naga-validate WGSL after MaterialXView TSL-portable conversion (same path as WebGPU viewer).
 *
 * Usage:
 *   node mxvalidategenwgsl_viewer.mjs --pixel path.frag.wgsl --naga path/to/naga
 *
 * Pixel stage only: matches WebGPU `fragment_*` shader modules (TSL wgslFn includes + entry).
 * Vertex uses TSL `varyings` plumbing and is not naga-checked here.
 */
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { spawnSync } from 'child_process';

import { convertToTslPortable } from '../../../javascript/MaterialXView/source/mxtsladapter.mjs';
import { buildWgslManifestFromWgsl } from '../../../javascript/MaterialXView/source/wgslmanifest.js';

const __dirname = path.dirname( fileURLToPath( import.meta.url ) );

function parseArgs( argv ) {

	const out = { pixel: null, vertex: null, naga: process.env.NAGA || 'naga' };
	for ( let i = 2; i < argv.length; i ++ ) {

		const a = argv[ i ];
		if ( a === '--pixel' && argv[ i + 1 ] ) out.pixel = argv[ ++ i ];
		else if ( a === '--vertex' && argv[ i + 1 ] ) out.vertex = argv[ ++ i ];
		else if ( a === '--naga' && argv[ i + 1 ] ) out.naga = argv[ ++ i ];

	}
	if ( ! out.pixel ) {

		console.error( 'mxvalidategenwgsl_viewer.mjs: --pixel is required' );
		process.exit( 2 );

	}
	return out;

}

function nagaValidate( naga, wgslText, stageFlag, label ) {

	const tmp = path.join( __dirname, `.parity_${ label }_${ process.pid }.wgsl` );
	try {

		fs.writeFileSync( tmp, wgslText, 'utf8' );
		const r = spawnSync( naga, [ '--input-kind', 'wgsl', '--shader-stage', stageFlag, tmp ], {
			encoding: 'utf8'
		} );
		if ( r.status !== 0 ) {

			const msg = ( r.stdout || '' ) + ( r.stderr || '' );
			return msg.trim() || `naga exit ${ r.status }`;

		}
		return '';

	} finally {

		try { fs.unlinkSync( tmp ); } catch { /* ignore */ }

	}

}

function validatePixelPortable( pixelWgsl, manifest, naga ) {

	const converted = convertToTslPortable( pixelWgsl, manifest );
	const combined = [ converted.includes, converted.entry ].filter( Boolean ).join( '\n' );
	if ( ! combined.trim() ) return 'empty TSL-portable pixel module';

	return nagaValidate( naga, combined, 'frag', 'pixel' );

}

const args = parseArgs( process.argv );
const pixelWgsl = fs.readFileSync( args.pixel, 'utf8' );
const vertexPath = args.vertex && fs.existsSync( args.vertex ) ? args.vertex : null;
const vertexWgsl = vertexPath ? fs.readFileSync( vertexPath, 'utf8' ) : '';

const manifest = buildWgslManifestFromWgsl( pixelWgsl, vertexWgsl || null );

const fragErr = validatePixelPortable( pixelWgsl, manifest, args.naga );
if ( fragErr ) {

	console.error( fragErr );
	process.exit( 1 );

}

process.exit( 0 );
