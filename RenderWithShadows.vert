// #version 330 compatibility
uniform mat4 uLightSpaceMatrix;
uniform mat4 uAnim;
uniform mat4 uModelView;
uniform mat4 uProj;
uniform float uLightX;
uniform float uLightY;
uniform float uLightZ;
varying vec4 vFragPosLightSpace;
varying vec3 vNs;
varying vec3 vLs;
varying vec3 vEs;
void
main()
{
    vec3 LightPosition = vec3(uLightX, uLightY, uLightZ);
    vec4 ECposition = uModelView * uAnim * gl_Vertex;
    vNs = normalize( mat3(uAnim) * gl_Normal );
    vLs = LightPosition - ECposition.xyz;
    vEs = vec3( 0., 0., 0. ) - ECposition.xyz;
    vFragPosLightSpace = uLightSpaceMatrix * uAnim * gl_Vertex;
    gl_Position = uProj * uModelView * uAnim * gl_Vertex;
}