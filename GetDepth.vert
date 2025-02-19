// #version 330 compatibility
uniform mat4 uLightSpaceMatrix;
uniform mat4 uAnim;
void
main()
{
    gl_Position = uLightSpaceMatrix * uAnim * gl_Vertex;
} 