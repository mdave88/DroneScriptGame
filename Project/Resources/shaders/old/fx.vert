
varying vec3 lightDir;
varying vec3 halfVec;
varying vec3 viewDir;

varying vec4 pos;
varying vec3 norm;

void main(){
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

	lightDir = gl_LightSource[0].position.xyz - pos.xyz;
	lightDir *= tbnMatrix;
	
	halfVec = gl_LightSource[0].halfVector.xyz;
	halfVec *= tbnMatrix;
	
	viewDir = -vec3(gl_ModelViewMatrix * gl_Vertex);
	viewDir *= tbnMatrix;
	
	gl_Position = ftransform();
}
