
varying vec3 N;
varying vec3 R;
varying vec3 T;
varying vec3 F;

varying vec3 x;

uniform sampler2D tex;
uniform samplerCube environmentMap;

vec3 HitParallax(vec3 x2, vec3 R2) {
	R2 = normalize(R2);
	
    float rl = textureCube(environmentMap, R2).a; // |r|
    
    float dp = rl - dot(x2, R2);
    vec3 p = x2 + R2 * dp;
    
    return p;
}

void main() {
	// Fetch reflected environment color
	//vec3 reflectedColor = textureCube(environmentMap, N).xyz * F;
	vec3 reflectedColor = textureCube(environmentMap, HitParallax(x, R)).xyz * F;
		
	//vec3 refractedColor = textureCube(environmentMap, T).xyz * (1.0 - F);
	vec3 refractedColor = vec3(0.0);//textureCube(environmentMap, HitParallax(x, T)).xyz * (1.0 - F);
	
	gl_FragColor = vec4(reflectedColor + refractedColor, 1.0);
}
