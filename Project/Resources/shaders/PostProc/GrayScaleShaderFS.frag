	
void main()
{
	vec3 color = texture2D(s_colorBuffer, v_texcoordL).rgb;
	float brightness = 0.3 * color.r + 0.59 * color.g + 0.11 * color.b;
	
	gl_FragColor = vec4(brightness, brightness, brightness, 1.0);
}
