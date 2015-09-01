// model attributes

// vertex data
attribute vec3	a_posL;
attribute vec3	a_normalL;
attribute vec3	a_tangentL;
attribute vec3	a_bitangentL;
attribute vec2	a_texcoordL;
attribute vec2	a_lightMapTexcoordL;


// vertex data
varying vec3	v_posL;
varying vec4	v_posW;
varying vec3	v_normalL;
varying vec3	v_tangentL;
varying vec3	v_bitangentL;
varying vec2	v_texcoordL;
varying vec2	v_lightMapTexcoordL;

varying vec3	v_normalW;
varying vec3	v_tangentW;
varying vec3	v_bitangentW;

// matrices
uniform mat4	u_M_Mat;
uniform mat4	u_MV_Mat;
uniform mat4	u_MVP_Mat;
uniform mat4	u_N_Mat;


// eye
uniform vec3	u_eyePositionW;
varying vec3	v_viewW;
varying vec3	v_viewTBN;
varying vec3	v_viewTBN2;

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

varying vec4	v_shadowCoords[8];


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
