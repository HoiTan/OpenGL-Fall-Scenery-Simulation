// make this 120 for the mac:
// #version 330 compatibility

// out variables to be interpolated in the rasterizer and sent to each fragment shader:
uniform mat4 uLightSpaceMatrix;
uniform mat4 uAnim;
uniform mat4 uModelView;
uniform mat4 uProj;
uniform float uLightX;
uniform float uLightY;
uniform float uLightZ;

varying vec4 vFragPosLightSpace;
// leaf part:
varying  vec2  vST;	  // (s,t) texture coordinates
varying  vec3  vN;	  // normal vector
varying  vec3  vL;	  // vector from point to light
varying  vec3  vE;	  // vector from point to eye

// where the light is:

// const vec3 LightPosition = vec3(  10., 20., 0. );

void
main( )
{
	vec3 LightPosition = vec3(uLightX, uLightY, uLightZ);
    vec4 ECposition = uModelView * uAnim * gl_Vertex;
	vST = gl_MultiTexCoord0.st;

	vN = normalize( gl_NormalMatrix * gl_Normal );  // normal vector
	vN = normalize(mat3(uAnim[0].xyz, uAnim[1].xyz, uAnim[2].xyz) * gl_Normal );
	vL = LightPosition - ECposition.xyz;	    // vector from the point
							// to the light position
	vE = vec3( 0., 0., 0. ) - ECposition.xyz;       // vector from the point
							// to the eye position
	// gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_Position = uProj * uModelView * uAnim * gl_Vertex;
    vFragPosLightSpace = uLightSpaceMatrix * uAnim * gl_Vertex;
    
	
}
