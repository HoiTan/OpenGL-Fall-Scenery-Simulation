// #version 330 compatibility
uniform vec3 uColor;
uniform sampler2D uShadowMap;
uniform int uShadowsOn;
varying vec4 vFragPosLightSpace;
varying vec3 vNs;
varying vec3 vLs;
varying vec3 vEs;
const float BIAS = 0.01;
const vec3 SPECULAR_COLOR = vec3(1.0, 1.0, 1.0);
const float SHININESS = 8.0;
const float KA = 0.20;
const float KD = 0.60;
const float KS = (1.0 - KA - KD);

bool CheckShadow(vec4 fragPosLightSpace)
{
    vec3 projection = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projection = 0.5 * projection + 0.5; // Normalize from [-1,1] to [0,1]

    // Ensure coordinates are inside valid range
    if (projection.x < 0.0 || projection.x > 1.0 || projection.y < 0.0 || projection.y > 1.0)
        return false;

    // Fetch the depth from the shadow map
    float closestDepth = texture2D(uShadowMap, projection.xy).r;

    // Get the fragment's depth
    float currentDepth = projection.z;

    // Check if in shadow
    return (currentDepth - BIAS) > closestDepth;
}

void main()
{
    vec3 normal = normalize(vNs);
    vec3 light = normalize(vLs);
    vec3 eye = normalize(vEs);
    float d = 0.0;
    float s = 0.0;
    vec3 lighting = KA * uColor;
    
    // Use a different name to avoid function-variable conflict
    bool inShadow = CheckShadow(vFragPosLightSpace);

    if (uShadowsOn == 0) // If shadows are disabled, ignore shadow calculations
        inShadow = false;

    if (!inShadow)
    {
        d = dot(normal, light);
        if (d > 0.0)
        {
            vec3 diffuse = KD * d * uColor;
            lighting += diffuse;
            vec3 refl = normalize(reflect(-light, normal));
            float dd = dot(eye, refl);
            if (dd > 0.0)
            {
                s = pow(dd, SHININESS);
                vec3 specular = KS * s * SPECULAR_COLOR;
                lighting += specular;
            }
        }
    }

    gl_FragColor = vec4(lighting, 1.0);
}
