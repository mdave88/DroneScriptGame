#version 330

// model attributes

// vertex data
layout (location = 0) in vec3	a_posL;
layout (location = 1) in vec2	a_texcoordL;
layout (location = 2) in vec3	a_normalL;
layout (location = 3) in vec3	a_tangentL;
layout (location = 4) in vec3	a_bitangentL;
layout (location = 5) in vec2	a_lightMapTexcoordL;
layout (location = 6) in ivec4	a_boneIds;
layout (location = 7) in vec4	a_boneWeights;

// vertex data
out vec3		v_posL;
out vec4		v_posW;
out vec3		v_normalL;
out vec3		v_tangentL;
out vec3		v_bitangentL;
out vec2		v_texcoordL;
out vec2		v_lightMapTexcoordL;

out vec3		v_normalW;
out vec3		v_tangentW;
out vec3		v_bitangentW;

// matrices
uniform mat4	u_M_Mat;
uniform mat4	u_MV_Mat;
uniform mat4	u_MVP_Mat;
uniform mat4	u_N_Mat;


// eye
uniform vec3	u_eyePositionW;
out vec3		v_viewW;
out vec3		v_viewTBN;
out vec3		v_viewTBN2;

// shadows
uniform int		u_enableShadows;

#define			LIGHTTYPE_DIRECTIONAL	0
#define			LIGHTTYPE_POINT			1
#define			LIGHTTYPE_SPOT			2

struct Light {
	int type;
	vec3 pos;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 attenuation;	//constant, linear, quadric

	// shadow computation
	mat4		shadowMatrix;
};

uniform int		u_numLightSources;
uniform Light	u_lights[8];

out vec4		v_shadowCoords[8];


void initAttribsL() {
	v_posL				= a_posL;
	v_normalL			= a_normalL;
	v_tangentL			= a_tangentL;
	v_bitangentL		= a_bitangentL;
	v_texcoordL			= a_texcoordL;
	v_lightMapTexcoordL	= a_lightMapTexcoordL;

	// for (int i = 0; i < u_numLightSources; i++) {
		// if (u_lights[i].type == LIGHTTYPE_SPOT)
			// v_shadowCoords[i] = u_lights[i].shadowMatrix * u_M_Mat * vec4(a_posL, 1.0);
	// }
}

void initAttribsW() {
	v_posW			= u_M_Mat * vec4(a_posL, 1.0);
	v_viewW			= v_posW.xyz - u_eyePositionW;

	v_normalW		= normalize( (u_N_Mat * vec4(a_normalL, 0.0)).xyz );
	v_tangentW		= normalize( (u_N_Mat * vec4(a_tangentL, 0.0)).xyz );
	v_bitangentW	= normalize( (u_N_Mat * vec4(a_bitangentL, 0.0)).xyz );

	mat3 TBN		= mat3(v_tangentW, v_bitangentW, v_normalW);
	v_viewTBN		= (-vec3(v_viewW)) * TBN;
}
