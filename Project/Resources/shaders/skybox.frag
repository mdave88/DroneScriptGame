
varying vec3 pos;
varying vec3 x;

uniform sampler2D tex;
uniform samplerCube environmentMap;
uniform vec3 eyePosition;

vec3 HitParallax(vec3 x2, vec3 R2) {
	R2 = normalize(R2);
	
    float rl = textureCube(environmentMap, R2).a; // |r|
    
    float dp = rl - dot(x2, R2);
    vec3 p = x2 + R2 * dp;
    
    return p;
}

void main() {
	vec3 color = textureCube(environmentMap, gl_TexCoord[1].xyz).rgb;
	
	gl_FragColor = vec4(color, 1.0) * 0.3;
}
