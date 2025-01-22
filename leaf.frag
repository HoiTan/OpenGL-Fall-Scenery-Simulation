// #version 120 compatibility

uniform float   uKa, uKd, uKs;		// coefficients of each type of lighting
uniform float   uShininess;			// specular exponent
uniform sampler2D uTexUnit;

varying  vec2  vST;			// texture coords
varying  vec3  vN;			// normal vector
varying  vec3  vL;			// vector from point to light
varying  vec3  vE;			// vector from point to eye

const float EYES = 0.91;					// not correct!
const float EYET = 0.65;					// not correct!
const float R 				= 0.03;				// radius of salmon eye
const vec3 SALMONCOLOR		= vec3( 0.98, 0.50, 0.45 );	// "salmon" (r,g,b) color
const vec3 EYECOLOR			= vec3( 0., 1., 0. );		// color to make the eye
const vec3 SPECULARCOLOR 	= vec3( 1., 1., 1. );

void
main( )
{
	vec3 newcolor = texture( uTexUnit, vST ).rgb;
	gl_FragColor = vec4( newcolor, 1. );
}
