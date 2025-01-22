#version 330 compatibility
uniform vec4 uColor;
uniform bool uSilh;
uniform float uDeltaZ;
uniform bool uOpaque;
uniform float uTol;
uniform float uKa, uKd, uKs; // coefficients of each type of lighting
uniform float uShininess; // specular exponent
in vec2 vST; // texture coords
in vec3 vN; // normal vector
in vec3 vL; // vector from point to light
in vec3 vE; // vector from point to eye
in float vNz;
const vec3 SPECULARCOLOR = vec3( 1., 1., 1. );
in vec3 SILHCOLOR = vec3( 0., 1., 0. );
void
main()
{
	vec3 myColor = uColor.rgb;
	// per-fragment lighting:
	vec3 Normal = normalize(vN);
	vec3 Light = normalize(vL);
	vec3 Eye = normalize(vE);
	vec3 ambient = uKa * myColor;
	float d = max( dot(Normal,Light), 0. ); // only do diffuse if the light can see the point
	vec3 diffuse = uKd * d * myColor;
	float s = 0.;
	if( d > 0. ) // only do specular if the light can see the point
	{
		vec3 ref = normalize( reflect( -Light, Normal ) );
		float cosphi = dot( Eye, ref );
		if( cosphi > 0. )
			s = pow( max( cosphi, 0. ), uShininess );
	}
	vec3 specular = uKs * s * SPECULARCOLOR.rgb;
	float deltaz = 0.;
	if( uSilh && ( abs(vNz) <= uTol ) )
	{
		gl_FragColor = vec4( SILHCOLOR, 1. );
		deltaz = -uDeltaZ;
	}
	else
	{
		if( ! uOpaque )
			discard;
		gl_FragColor = vec4( ambient + diffuse + specular, 1. );
	}
	gl_FragDepth = gl_FragCoord.z + deltaz;
}