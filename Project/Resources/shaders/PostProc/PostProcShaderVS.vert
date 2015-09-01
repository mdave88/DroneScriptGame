
void main()
{
	vec2 position;
	position.xy = sign(a_posL.xy);
	v_texcoordL = position.xy *.5 + .5;
	
	gl_Position = vec4(position.xy, 0.0, 1.0);
}
