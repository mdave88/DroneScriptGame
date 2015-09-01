const int MAX_BONE_COUNT = 100;

uniform mat4 gWVP;
uniform mat4 gWorld;
uniform mat4 u_bones[MAX_BONE_COUNT];

void main()
{
	mat4 boneTransform	=  u_bones[a_boneIds[0]] * a_boneWeights[0];
	boneTransform		+= u_bones[a_boneIds[1]] * a_boneWeights[1];
	boneTransform		+= u_bones[a_boneIds[2]] * a_boneWeights[2];
	boneTransform		+= u_bones[a_boneIds[3]] * a_boneWeights[3];

	vec4 posL			= boneTransform * vec4(a_posL, 1.0);
	vec4 normalL		= boneTransform * vec4(a_normalL, 0.0);
	vec4 tangentL		= boneTransform * vec4(a_tangentL, 0.0);
	vec4 bitangentL		= boneTransform * vec4(a_bitangentL, 0.0);

	v_posW				= u_M_Mat * posL;
	v_normalW			= normalize(u_N_Mat * normalL).xyz;
	v_tangentW			= normalize(u_N_Mat * tangentL).xyz;
	v_bitangentW		= normalize(u_N_Mat * bitangentL).xyz;

	gl_Position  = u_MVP_Mat * posL;
}
