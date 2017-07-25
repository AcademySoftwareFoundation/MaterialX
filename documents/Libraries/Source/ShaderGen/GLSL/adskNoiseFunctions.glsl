void hash2d(vec2 gridcell, out vec4 hash_0, out vec4 hash_1)
{
	const vec2 OFFSET = vec2( 26.0, 161.0 );
	const float DOMAIN = 71.0;
	const vec2 SOMELARGEFLOATS = vec2( 951.135664, 642.949883 );
	vec4 P = vec4( gridcell.xy, gridcell.xy + 1.0 );
	P = P - floor(P * ( 1.0 / DOMAIN )) * DOMAIN;
	P += OFFSET.xyxy;
	P *= P;
	P = P.xzxz * P.yyww;
	hash_0 = fract( P * ( 1.0 / SOMELARGEFLOATS.x ) );
	hash_1 = fract( P * ( 1.0 / SOMELARGEFLOATS.y ) );
}

vec3 invsqrt( vec3 input_is_glsl_kw )
{
	return vec3(1.0f / sqrt(input_is_glsl_kw));
}

float simplex_perlin2d( vec2 P )
{
	//	simplex math constants
	const float SKEWFACTOR = 0.36602540378443864676372317075294;			// 0.5*(sqrt(3.0)-1.0)
	const float UNSKEWFACTOR = 0.21132486540518711774542560974902;			// (3.0-sqrt(3.0))/6.0
	const float SIMPLEX_TRI_HEIGHT = 0.70710678118654752440084436210485;	// sqrt( 0.5 )	height of simplex triangle
	const vec3 SIMPLEX_POINTS = vec3( 1.0-UNSKEWFACTOR, -UNSKEWFACTOR, 1.0-2.0*UNSKEWFACTOR );	// vertex info for simplex triangle

	//	establish our grid cell.
	P *= SIMPLEX_TRI_HEIGHT;		// scale space so we can have an approx feature size of 1.0  ( optional )
	vec2 Pi = floor( P + dot( P, vec2( SKEWFACTOR ) ) );

	//	calculate the hash.
	vec4 hash_x, hash_y;
	hash2d( Pi, hash_x, hash_y );

	//	establish vectors to the 3 corners of our simplex triangle
	vec2 v0 = Pi - dot( Pi, vec2( UNSKEWFACTOR ) ) - P;

	vec4 v1pos_v1hash = (v0.x < v0.y) ? vec4(SIMPLEX_POINTS.xy, hash_x.y, hash_y.y) : vec4(SIMPLEX_POINTS.yx, hash_x.z, hash_y.z);
	vec4 v12 = vec4( v1pos_v1hash.xy, SIMPLEX_POINTS.zz ) + v0.xyxy;

	//	calculate the dotproduct of our 3 corner vectors with 3 random normalized vectors
	vec3 grad_x = vec3( hash_x.x, v1pos_v1hash.z, hash_x.w ) - 0.49999;
	vec3 grad_y = vec3( hash_y.x, v1pos_v1hash.w, hash_y.w ) - 0.49999;
	vec3 grad_results = invsqrt( grad_x * grad_x + grad_y * grad_y ) * ( grad_x * vec3( v0.x, v12.xz ) + grad_y * vec3( v0.y, v12.yw ) );

	//	Normalization factor to scale the final result to a strict 1.0->-1.0 range
	//	x = ( sqrt( 0.5 )/sqrt( 0.75 ) ) * 0.5
	//	NF = 1.0 / ( x * ( ( 0.5 – x*x ) ^ 4 ) * 2.0 )
	//	http://briansharpe.wordpress.com/2012/01/13/simplex-noise/#comment-36
	const float FINAL_NORMALIZATION = 99.204334582718712976990005025589;

	//	evaluate the surflet, sum and return
	vec3 m = vec3( v0.x, v12.xz ) * vec3( v0.x, v12.xz ) + vec3( v0.y, v12.yw ) * vec3( v0.y, v12.yw );
	m = max(0.5 - m, 0.0);		//	The 0.5 here is SIMPLEX_TRI_HEIGHT^2
	m = m*m;
	return dot(m*m, grad_results) * FINAL_NORMALIZATION;
}

vec4 cellular_weight_samples( vec4 samples )
{
	samples = samples * 2.0 - 1.0;
	return (samples * samples * samples) - sign(samples);	// cubic (even more variance)
}

float simplex_cellular2d( vec2 P )
{
	//	simplex math based off Stefan Gustavson's and Ian McEwan's work at...
	//	http://github.com/ashima/webgl-noise

	//	simplex math constants
	const float SKEWFACTOR = 0.36602540378443864676372317075294;			// 0.5*(sqrt(3.0)-1.0)
	const float UNSKEWFACTOR = 0.21132486540518711774542560974902;			// (3.0-sqrt(3.0))/6.0
	const float SIMPLEX_TRI_HEIGHT = 0.70710678118654752440084436210485;	// sqrt( 0.5 )	height of simplex triangle.
	const float INV_SIMPLEX_TRI_HEIGHT = 1.4142135623730950488016887242097;	//	1.0 / sqrt( 0.5 )
	const vec3 SIMPLEX_POINTS = vec3( 1.0-UNSKEWFACTOR, -UNSKEWFACTOR, 1.0-2.0*UNSKEWFACTOR ) * INV_SIMPLEX_TRI_HEIGHT;	// vertex info for simplex triangle

	//	establish our grid cell.
	P *= SIMPLEX_TRI_HEIGHT;		// scale space so we can have an approx feature size of 1.0  ( optional )
	vec2 Pi = floor( P + dot( P, vec2( SKEWFACTOR ) ) );

	//	calculate the hash.
	vec4 hash_x, hash_y;
	hash2d( Pi, hash_x, hash_y );

	//	push hash values to extremes of jitter window
	const float JITTER_WINDOW = ( 0.10566243270259355887271280487451 * INV_SIMPLEX_TRI_HEIGHT ); // this will guarentee no artifacts.
	hash_x = cellular_weight_samples( hash_x ) * JITTER_WINDOW;
	hash_y = cellular_weight_samples( hash_y ) * JITTER_WINDOW;

	//	calculate sq distance to closest point
	vec2 p0 = ( ( Pi - dot( Pi, vec2( UNSKEWFACTOR ) ) ) - P ) * INV_SIMPLEX_TRI_HEIGHT;
	hash_x += p0.xxxx;
	hash_y += p0.yyyy;
	hash_x.yzw += SIMPLEX_POINTS.xyz;
	hash_y.yzw += SIMPLEX_POINTS.yxz;
	vec4 distsq = hash_x*hash_x + hash_y*hash_y;
	vec2 tmp = min( distsq.xy, distsq.zw );
	return min( tmp.x, tmp.y );
}
