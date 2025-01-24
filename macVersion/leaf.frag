// make this 120 for the mac:
// #version 330 compatibility

// lighting uniform variables -- these can be set once and left alone:
uniform float   uKa, uKd, uKs;	 // coefficients of each type of lighting -- make sum to 1.0
uniform vec4    uColor;		 // object color
uniform vec4    uSpecularColor;	 // light color
uniform float   uShininess;	 // specular exponent
uniform float   uAlpha;		 // transparency

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
	float s = vST.s;
	float t = vST.t;

	// determine the color using the square-boundary equations:

	vec3 myColor = uColor.rgb;

	// apply the per-fragmewnt lighting to myColor:

	vec3 Normal = normalize(vN);
	vec3 Light  = normalize(vL);
	vec3 Eye    = normalize(vE);

	vec3 ambient = uKa * myColor;

	float dd = max( dot(Normal,Light), 0. );       // only do diffuse if the light can see the point
	vec3 diffuse = uKd * dd * myColor;

	float ss = 0.;
	if( dot(Normal,Light) > 0. )	      // only do specular if the light can see the point
	{
		vec3 ref = normalize(  reflect( -Light, Normal )  );
		ss = pow( max( dot(Eye,ref),0. ), uShininess );
		vec3 specular = uKs * ss * uSpecularColor.rgb;
		gl_FragColor = vec4( ambient + diffuse + specular, uAlpha );
	}
	else if( dot(-Normal,Light) > 0.)
	{
		// Calculate translucency color by subtracting light color from object color
		float translucencyFactor = max(dot(-Normal, Light), 0.0);  // Back-facing light
		vec3 translucencyColor = max(uColor.rgb - uSpecularColor.rgb, 0.0);  // Ensure no negative colors
		vec3 translucency = translucencyFactor * translucencyColor;

		// Combine diffuse and translucency
		vec3 finalColor = diffuse + translucency;
		gl_FragColor = vec4( finalColor, uAlpha );

	}
	

	

	
	// gl_FragColor = vec4( ambient + diffuse + specular, uAlpha );
}

