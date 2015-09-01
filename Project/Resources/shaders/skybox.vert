
varying vec3 positionW;
varying vec3 x;

uniform vec3 objPos;
uniform vec3 eyePositionW;

// if there is a problem with uniform mat4 -> load the matrix by rows
uniform vec4 mtw0;
uniform vec4 mtw1;
uniform vec4 mtw2;
uniform vec4 mtw3;


void main() {
	mat4 modelToWorld = mat4(mtw0, mtw1, mtw2, mtw3);
	mat3 modelToWorld3x3 = mat3(mtw0.xyz, mtw1.xyz, mtw2.xyz);

	positionW = vec3(modelToWorld * gl_Vertex);

	vec3 x = positionW - objPos;

	vec3 N = normalize(modelToWorld3x3 * gl_Normal);
	vec3 E = normalize(positionW - eyePositionW);

	gl_TexCoord[1].xyz = reflect(E, N);
	gl_Position = ftransform();
}
