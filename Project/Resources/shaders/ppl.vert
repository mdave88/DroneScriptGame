
// shadows
uniform int enableShadows;
uniform int enabledLights[10];
varying vec4 shadowCoord;

// lights
varying vec3 viewDir;
varying vec3 lightDirs[10];

varying vec4 pos;
varying vec3 norm;

//struct PointLight {
	//bool enabled;
//
	//vec3 pos;
//
	//vec3 ambient;
	//vec3 diffuse;
	//vec3 specular;
//
	//vec3 attenuation;
//};

void applyLight(int i, mat3 tbnMatrix) {
	if (enabledLights[i] > 0) {
		lightDirs[i] = (gl_LightSource[i].position.xyz - pos.xyz) * tbnMatrix;
	}
}

void main() {
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;

	pos = gl_ModelViewMatrix * gl_Vertex;
	norm = gl_NormalMatrix * gl_Normal;

	
	vec3 t = normalize(gl_NormalMatrix * gl_MultiTexCoord1.xyz);
	vec3 b;
	vec3 n = normalize(gl_NormalMatrix * gl_Normal);

	// if bitangents are set
	if (length(gl_MultiTexCoord2.xyz) > 0.0) {
		b = normalize(gl_NormalMatrix * gl_MultiTexCoord2.xyz);
	} else {
		b = normalize(cross(n, t));
	}

	mat3 tbnMatrix = mat3(t, b, n);

	for (int i = 0; i < 8; i++) {	// max 8 light are allowed by opengl
		applyLight(i, tbnMatrix);
	}
	
	viewDir = (-vec3(pos)) * tbnMatrix;
	

	if (enableShadows > 0)
		shadowCoord= gl_TextureMatrix[7] * gl_Vertex;
}
