
varying vec3 distance;
uniform sampler2D tex;

void main() {
	vec3 color = vec3(texture2D(tex, gl_TexCoord[0].st));
	gl_FragColor = vec4(color, length(distance));
}
