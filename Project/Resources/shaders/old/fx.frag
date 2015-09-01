
varying vec4  pos;
varying vec3  norm;

uniform sampler2D tex;
uniform sampler2D normalHeightMap;

uniform int technique;
uniform float scale;
uniform float bias;

uniform bool enableParallax;

varying vec3 lightDir;
varying vec3 halfVec;
varying vec3 viewDir;

uniform bool light0_enabled;
uniform float elapsedTime;

float rand(vec2 co){
	//return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453 * sin(elapsedTime));
}

vec2 techniqueBase() {
	return gl_TexCoord[0].st;
}

vec2 techniqueParallax() {
	float height;
	vec3 v = normalize(viewDir);
	vec2 uv = gl_TexCoord[0].st;

	vec4 N = texture2D(normalHeightMap, gl_TexCoord[0].st);
	height = N.a * scale + bias;
	uv = height * v.xy + gl_TexCoord[0].st;

	return uv;
}

vec2 techniqueBinary() {
	float height;
	vec3 v = normalize(viewDir);
	vec2 uv = gl_TexCoord[0].st;

	float H, Hmin = 0.0, Hmax = 1.0;
		
	vec2 uvin = v.xy + gl_TexCoord[0].st;
	vec2 uvout = gl_TexCoord[0].st;
		
	for (int i = 0; i < 10; i++) {
		H = (Hmin + Hmax) / 2.0;   // middle
		uv = uvin * H + uvout * (1.0 - H);
			
		height = texture2D(normalHeightMap, uv).a * scale * 1.5 + bias * 1.5;
			
		if (H <= height) {
			Hmin = H; // below
		} else {
			Hmax = H; // above
		}
	}

	return uv;
}

vec4 computeLights() {

}

 
void main() {
	vec2 uv;

	if (technique == 0) {
		uv = techniqueParallax();
	} else if (technique == 1) {
		uv = techniqueBinary();
	} else {
		uv = techniqueBase();
	}

	vec4 finalcolor = (gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient);
	
	if (light0_enabled){
		float d  = length(vec3(gl_LightSource[0].position - pos));
		float att = 1.00 / (gl_LightSource[0].constantAttenuation + 
				(gl_LightSource[0].linearAttenuation*d) + 
				(gl_LightSource[0].quadraticAttenuation*d*d));

		finalcolor += (gl_LightSource[0].ambient * gl_FrontMaterial.ambient) * att;

		vec3 normal;
		vec3 normData;
		vec3 lightDirection;

		if (length(normData = texture2D(normalHeightMap, uv).xyz) == 0.0) {
			// no bumpmap given -> use the world space vectors
			normal = normalize(norm);
			lightDirection = vec3(gl_LightSource[0].position - pos);
		} else {
			// bumpmap given -> use the tangent space vectors
			normal = normalize(normData * 2.0 - 1.0);
			lightDirection = lightDir;
		}

		float NdotL = max(dot(normal, normalize(lightDirection)), 0.0);

		if (NdotL > 0.0){
			finalcolor += gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * NdotL * att;		
		}
	}
	
	gl_FragColor = texture2D(tex, uv) * finalcolor;
	//gl_FragColor = finalcolor;
	//gl_FragColor = texture2D(tex, gl_TexCoord[0].st);
	//gl_FragColor = texture2D(Texture, gl_TexCoord[0].st) * finalcolor * vec4(rand(gl_TexCoord[0].xy), rand(gl_TexCoord[0].xy), rand(gl_TexCoord[0].xy), 1);
}
