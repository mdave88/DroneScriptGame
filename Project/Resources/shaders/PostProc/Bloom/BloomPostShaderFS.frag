
void main()
{
	 vec4 color = texture2D(s_colorBuffer, v_texcoordL);
	 vec4 blur = texture2D(s_blurBuffer, v_texcoordL);
	 
	gl_FragColor = addSmooth(color, blur * 2.0);
}
