
uniform float u_near;
uniform float u_far;
uniform float u_fadeDistance;
uniform float u_clarity;

void main()
{
	vec3  color	= texture2D(s_colorBuffer, v_texcoordL).rgb;
	vec3  blur	= texture2D(s_blurBuffer, v_texcoordL).rgb;
	float depth	= texture2D(s_depthBuffer, v_texcoordL).r;
	
	//float z = (2.0 * u_near) / (u_far + u_near - depth * (u_far - u_near));
	float z = -u_far * u_near / (depth * (u_far - u_near) - u_far);
	
	float k = clamp(abs(z - u_clarity) / u_fadeDistance, 0.0, 1.0);
	
	gl_FragColor = vec4( mix(color, blur, k), 1.0 );
	//gl_FragColor = vec4(color, 1.0 );
}
