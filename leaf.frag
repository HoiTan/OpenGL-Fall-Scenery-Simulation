// make this 120 for the mac:
// #version 330 compatibility

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


void 
main( )
{
    // Basic color for this object (no textures are being used here, 
    // but you can sample a texture if you want)
    vec3 myColor = uColor.rgb;
    // vec3 myColor = vec3(0, 1, 0);

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
    vec3 specularColor = vec3( 1.0, 1.0, 1.0 );
    vec3 specular = uKs * ss * specularColor.rgb;

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
