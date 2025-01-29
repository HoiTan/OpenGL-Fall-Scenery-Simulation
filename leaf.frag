#version 120

// Lighting uniform variables:
uniform float   uKa, uKd, uKs;     // Coefficients for ambient, diffuse, specular
uniform vec4    uColor;            // Object (diffuse) color
uniform vec4    uSpecularColor;    // Specular highlight color
uniform float   uShininess;        // Specular exponent
uniform float   uAlpha;            // Transparency (alpha)

// Extra lighting param for translucency strength:
uniform float   uTranslucency;     // How strong the back-light effect is

// "Square-equation" uniforms (not explicitly used here, but left for completeness):
uniform float   uS0, uT0, uD;

// Interpolated from the vertex shader:
varying vec3    vN;                // Normal vector at this fragment
varying vec3    vL;                // Vector from fragment to light
varying vec3    vE;                // Vector from fragment to eye
varying vec2    vST;               // (s,t) texture coordinates

void main( )
{
    // Basic color for this object (no textures are being used here, 
    // but you can sample a texture if you want)
    vec3 myColor = uColor.rgb;

    // Normalize the interpolated vectors:
    vec3 Normal = normalize( vN );
    vec3 Light  = normalize( vL );
    vec3 Eye    = normalize( vE );

    // 1. Ambient Term:
    vec3 ambient = uKa * myColor;

    // 2. Diffuse Term:
    float ndotl   = max( dot(Normal, Light), 0.0 );
    vec3  diffuse = uKd * ndotl * myColor;

    // 3. Specular Term:
    float ss = 0.0;
    if( ndotl > 0.0 )    // only add specular if the fragment faces the light
    {
        // reflect( -Light, Normal ) is the reflection vector
        vec3 reflectDir = reflect( -Light, Normal );
        float edotr      = max( dot(Eye, reflectDir), 0.0 );
        ss              = pow( edotr, uShininess );
    }
    vec3 specular = uKs * ss * uSpecularColor.rgb;

    // 4. Translucency Term:
    //    How strongly the light hits the 'back' side:
    float backLit = max( dot(-Normal, Light), 0.0 );

    //    Use the same base color or a variation; multiply by "uTranslucency" to control strength:
    vec3 translucency = uTranslucency * backLit * myColor;

    // 5. Combine all terms:
    //    You can include ambient & specular if you like. For a classic Phong + translucency:
    vec3 finalColor = ambient + diffuse + specular + translucency;

    gl_FragColor = vec4( finalColor, uAlpha );
}
