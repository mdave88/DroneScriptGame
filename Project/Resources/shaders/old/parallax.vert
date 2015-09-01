
varying vec3 lightDir;
varying vec3 halfVector;
varying vec3 viewDir;

void main() {
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	
	vec3 t = normalize(gl_NormalMatrix * gl_MultiTexCoord1.xyz);
	vec3 b = normalize(gl_NormalMatrix * gl_MultiTexCoord2.xyz);
	vec3 n = normalize(gl_NormalMatrix * gl_Normal);
	
	mat3 tbnMatrix = mat3(t, b, n);
	
	lightDir = gl_LightSource[0].position.xyz;
	lightDir *= tbnMatrix;
	
	halfVector = gl_LightSource[0].halfVector.xyz;
	halfVector *= tbnMatrix;
	
	viewDir = -vec3(gl_ModelViewMatrix * gl_Vertex);
	viewDir *= tbnMatrix;
}
