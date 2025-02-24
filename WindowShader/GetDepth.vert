// #version 330 compatibility
// uniform vec3 uLightPos;

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
const vec3 LightPosition = vec3(  10., 10., 10. );
void
main()
{
    mat4 lightProjection = orthographic(-30.f, 30.f, -30.f, 30.f, 0.1f, 80.f);
    mat4 lightView = lookAt(LightPosition, vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
    mat4 lightSpaceMatrix = lightProjection * lightView;
    gl_Position = lightSpaceMatrix * gl_Vertex;
    //     gl_Position = lightSpaceMatrix * gl_Vertex * vec4(aPos, 1.0);
} 