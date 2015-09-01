
uniform float u_limit;

void main()
{
	vec3 color = texture2D(s_colorBuffer, v_texcoordL).rgb;
	float brightness = 0.3 * color.r + 0.59 * color.g + 0.11 * color.b;
	
	
	float val = step(u_limit, brightness);
	gl_FragColor = vec4(color * vec3(val), 1.0);
}
