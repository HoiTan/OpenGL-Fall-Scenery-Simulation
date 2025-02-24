#version 330 compatibility
// uniform vec3 uLightPos;

// uniform mat4 uAnim;
// uniform mat4 uModelView;
// uniform mat4 uProj;
uniform float uLightX;
uniform float uLightY;
uniform float uLightZ;
out vec4 vFragPosLightSpace;

// out variables to be interpolated in the rasterizer and sent to each fragment shader:
out  vec2  vST;	  // (s,t) texture coordinates
out  vec3  vN;	  // normal vector
out  vec3  vL;	  // vector from point to light
out  vec3  vE;	  // vector from point to eye
out vec3 vNs;
out vec3 vLs;
out vec3 vEs;

// where the light is:

// const vec3 LightPosition = vec3(  10., 10., 10. );

mat4 lookAt(vec3 eye, vec3 center, vec3 up)
{
    vec3 f = normalize(center - eye);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);

    mat4 result = mat4(1.0);

    result[0][0] = s.x;
    result[1][0] = s.y;
    result[2][0] = s.z;

    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;

    result[0][2] = -f.x;
    result[1][2] = -f.y;
    result[2][2] = -f.z;

    result[3][0] = -dot(s, eye);
    result[3][1] = -dot(u, eye);
    result[3][2] =  dot(f, eye);

    return result;
}

mat4 orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    mat4 result = mat4(1.0);
    result[0][0] = 2.0 / (right - left);
    result[1][1] = 2.0 / (top - bottom);
    result[2][2] = -2.0 / (farPlane - nearPlane);
    result[3][0] = - (right + left) / (right - left);
    result[3][1] = - (top + bottom) / (top - bottom);
    result[3][2] = - (farPlane + nearPlane) / (farPlane - nearPlane);
    return result;
}

void
main( )
{
	vec3 LightPosition = vec3(uLightX, uLightY, uLightZ);
	vST = gl_MultiTexCoord0.st;
	vec4 ECposition = gl_ModelViewMatrix * gl_Vertex;
	vN = normalize( gl_NormalMatrix * gl_Normal );  // normal vector
	vL = LightPosition - ECposition.xyz;	    // vector from the point
							// to the light position
	vE = vec3( 0., 0., 0. ) - ECposition.xyz;       // vector from the point
							// to the eye position
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	
	// vNs = normalize( mat3(uAnim) * gl_Normal );
	vNs = normalize( gl_Normal );
	vLs = LightPosition - ECposition.xyz;
    vEs = vec3( 0., 0., 0. ) - ECposition.xyz;

	mat4 lightProjection = orthographic(-30.f, 30.f, -30.f, 30.f, 0.1f, 80.f);
    mat4 lightView = lookAt(LightPosition, vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
    mat4 lightSpaceMatrix = lightProjection * lightView;
	// vFragPosLightSpace = lightSpaceMatrix * uAnim * gl_Vertex;
	vFragPosLightSpace = lightSpaceMatrix * gl_Vertex;
}
