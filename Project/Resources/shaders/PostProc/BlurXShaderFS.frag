
uniform vec2 u_pixelSize;

void main()
{
	vec2 samples0, samples1, samples2, samples3, samples4;
	vec4 rez;
	
	samples0 = v_texcoordL + u_pixelSize.x * vec2( 0.0, 0.0);
	samples1 = v_texcoordL + u_pixelSize.x * vec2(-1.0, 0.0);
	samples2 = v_texcoordL + u_pixelSize.x * vec2(-2.0, 0.0);
	samples3 = v_texcoordL + u_pixelSize.x * vec2( 1.0, 0.0);
	samples4 = v_texcoordL + u_pixelSize.x * vec2( 2.0, 0.0);
	
	rez =	0.061 * (texture2D(s_colorBuffer, samples1) + texture2D(s_colorBuffer, samples4)) +
			0.242 * (texture2D(s_colorBuffer, samples2) + texture2D(s_colorBuffer, samples3)) +
			0.383 *  texture2D(s_colorBuffer, samples0);

	gl_FragColor = rez;
}
