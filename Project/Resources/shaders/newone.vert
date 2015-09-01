
void main()
{
	initAttribsW();
	initAttribsL();

	gl_Position = u_MVP_Mat * vec4(a_posL, 1.0);
}
