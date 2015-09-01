
varying vec3 distance;

void main() {
	distance = vec3(gl_ModelViewMatrix * gl_Vertex);
	
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}
