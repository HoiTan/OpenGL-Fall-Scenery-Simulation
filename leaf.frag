// make this 120 for the mac:
// #version 330 compatibility

// shadow
uniform sampler2D uShadowMap;
uniform int uShadowsOn;
varying vec4 vFragPosLightSpace;
const float BIAS = 0.01;
const vec3 SPECULAR_COLOR = vec3( 1., 1., 1. );
const float SHININESS = 8.;
const float KA = 0.20;
const float KD = 0.60;
const float KS = (1.-KA-KD);

// lighting uniform variables -- these can be set once and left alone:
uniform float   uKa, uKd, uKs;	 // coefficients of each type of lighting -- make sum to 1.0
uniform vec3    uColor;		 // object color
uniform vec4    uSpecularColor;	 // light color
uniform float   uShininess;	 // specular exponent
uniform float   uAlpha;		 // transparency

// Extra lighting param for translucency strength:
uniform float   uTranslucency;     // How strong the back-light effect is

// square-equation uniform variables -- these should be set every time Display( ) is called:
uniform float   uS0, uT0, uD;

// in variables from the vertex shader and interpolated in the rasterizer:

varying  vec3  vN;		   // normal vector
varying  vec3  vL;		   // vector from point to light
varying  vec3  vE;		   // vector from point to eye
varying  vec2  vST;		   // (s,t) texture coordinates

bool IsInShadow(vec4 fragPosLightSpace)
{
    // have to manually do homogenous division to make light space position in range of -1 to 1:
    vec3 projection = fragPosLightSpace.xyz / fragPosLightSpace.w;
    //then make it from 0 to 1:
    projection = 0.5*projection + 0.5;
    //get closest depth from light's perspective
    float closestDepth = texture2D(uShadowMap, projection.xy).r;
    //get current depth:
    float currentDepth = projection.z;
    bool isInShadow = (currentDepth - BIAS) > closestDepth;
    return isInShadow;
}

void main()
{
    vec3 myColor = uColor;
    // vec3 myColor = vec3(0, 1, 0); // Object color (green)

    // Normalize the interpolated vectors:
    vec3 Normal = normalize(vN);
    vec3 Light  = normalize(vL);
    vec3 Eye    = normalize(vE);
    float d = 0.;
    float s = 0.;
    bool isInShadow = IsInShadow(vFragPosLightSpace);
    // isInShadow = false; // for now, just to see the effect of the lighting
    if( uShadowsOn != 0 )
        isInShadow = false; // if in ShadowOff mode, nothing should be cnsidered in a shadow
    
    if( ! isInShadow )
    {
        // 1. Ambient Term:
        vec3 ambient = uKa * myColor;

        // 2. Diffuse Term:
        float ndotl   = max(dot(Normal, Light), 0.0);
        vec3  diffuse = uKd * ndotl * myColor;

        // 3. Specular Term:
        float ss = 0.0;
        if (ndotl > 0.0) // Only add specular if the fragment faces the light
        {
            vec3 reflectDir = reflect(-Light, Normal);
            float edotr      = max(dot(Eye, reflectDir), 0.0);
            ss              = pow(edotr, uShininess);
        }
        vec3 specularColor = vec3(1.0, 1.0, 1.0);
        vec3 specular = uKs * ss * specularColor.rgb;

        // 4. Translucency Term:
        float backLit = max(dot(-Normal, Light), 0.0);
        vec3 translucency = uTranslucency * backLit * myColor;

        // 5. Adjust transparency based on back-lighting
        float finalAlpha = mix(uAlpha, uAlpha * 0.6, backLit); // Reduce alpha when back-lit

        // 6. Combine all terms:
        vec3 finalColor = ambient + diffuse + specular + translucency;

        gl_FragColor = vec4(finalColor, clamp(finalAlpha, 0.0, 1.0));
    }
}
