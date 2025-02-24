// make this 120 for the mac:
// #version 330 compatibility

// out variables to be interpolated in the rasterizer and sent to each fragment shader:
// leaf part:
varying  vec2  vST;	  // (s,t) texture coordinates
varying  vec3  vN;	  // normal vector
varying  vec3  vL;	  // vector from point to light
varying  vec3  vE;	  // vector from point to eye

// where the light is:

// const vec3 LightPosition = vec3(  10., 20., 0. );

#version 330 compatibility

void
main( )
{
	vST = gl_MultiTexCoord0.st;
	vec4 ECposition = gl_ModelViewMatrix * gl_Vertex;
	vN = normalize( gl_NormalMatrix * gl_Normal );  // normal vector
	vL = LightPosition - ECposition.xyz;	    // vector from the point
							// to the light position
	vE = vec3( 0., 0., 0. ) - ECposition.xyz;       // vector from the point
							// to the eye position
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
