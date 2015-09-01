
varying vec3 N;
varying vec3 R;

varying vec3 T;
varying vec3 F;

varying vec3 x;

uniform vec3 n;
uniform vec3 k;
uniform vec3 Fm;

uniform vec3 objPos;
uniform float time;

uniform mat4 modelToWorld;
uniform vec3 eyePositionW;

void computeFresnel(vec3 I, vec3 N) {
	float costheta = -dot(I, N);
/*
	vec3 Fm;
	vec3 k2 = k * k;
	Fm = ((n - vec3(1.0)) * (n - vec3(1.0)) + k2) / ((n + vec3(1.0)) * (n + vec3(1.0)) + k2);
*/
	F = Fm + (vec3(1.0) - Fm) * pow(1.0 - costheta, 5.0);
}


void main() {
	// Compute position and normal in world space
	vec3 positionW = vec3(modelToWorld * gl_Vertex);

	x = positionW - objPos;
	
	// Normal
	mat3 modelToWorld3x3 = mat3(modelToWorld[0].xyz, modelToWorld[1].xyz, modelToWorld[2].xyz);
	N = modelToWorld3x3 * gl_Normal.xyz;
	N = normalize(N);
	
	// Compute the incident and reflected vectors
	vec3 I = positionW - eyePositionW;
	I = normalize(I);
	
	computeFresnel(I, N);
	
	R = reflect(I, N);
	T = refract(-I, N, 0.47);
	
	gl_TexCoord[0] = gl_MultiTexCoord0;
	
	gl_Position = ftransform();
}

