#version 330
#extension GL_EXT_texture_array : enable

//precision mediump float;

// vertex local data
in vec3		v_posL;
in vec4		v_posW;
in vec3		v_normalL;
in vec3		v_tangentL;
in vec3		v_bitangentL;
in vec2		v_texcoordL;
in vec2		v_lightMapTexcoordL;

// vertex world data
in vec3		v_normalW;
in vec3		v_tangentW;
in vec3		v_bitangentW;


// default textures
uniform sampler2D	s_colorTex;
uniform sampler2D	s_lightMapTex;
uniform sampler2D	s_normalHeightTex;

uniform samplerCube s_cubeTex;


// additional textures
uniform sampler2D	s_tex1;
uniform sampler2D	s_tex2;
uniform sampler2D	s_tex3;
uniform sampler2D	s_tex4;

// fbo textures
uniform sampler2D	s_blurBuffer;
uniform sampler2D	s_colorBuffer;
uniform sampler2D	s_depthBuffer;


// light computation
uniform vec3		u_eyePositionW;
in vec3		v_viewW;
in vec3		v_viewTBN;
in vec3		v_viewTBN2;


// shadows

uniform int			u_enableShadows;

#define				LIGHTTYPE_DIRECTIONAL	0
#define				LIGHTTYPE_POINT			1
#define				LIGHTTYPE_SPOT			2

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

uniform int			u_numLightSources;
uniform Light		u_lights[8];
in vec4		v_shadowCoords[8];

uniform	float			u_shadowMapPixelOffsetX;
uniform	float			u_shadowMapPixelOffsetY;
uniform sampler2DArray	u_shadowMapArray;


// fog computation
uniform float		u_fogStart, u_fogLength;
uniform vec3		u_fogColor;


uniform float		u_elapsedTime;


// parallax mapping
uniform int			u_technique;
uniform int			u_techniqueMax;
uniform float		u_scale;
uniform float		u_bias;

vec4 addSmooth(in vec4 a, in vec4 b) {
	return a + b * (1.0 - a);
}

float lookupShadowDepth(in int lightId, in vec2 offSet) {
	//vec3 normal = normalize(v_normalW);
	//vec3 lightDir = v_posW.xyz - u_lights[1].pos;
	//float cosTheta = dot(normal, -normalize(lightDir));
	//float bias = 0.005 * tan(acos(cosTheta));
	//bias = clamp(bias, 0, 0.01);

	float bias = 0.005;
	vec2 offsetVec = vec2(offSet.x * u_shadowMapPixelOffsetX, offSet.y * u_shadowMapPixelOffsetY);
	
	float shadowDepth = texture2DArray(u_shadowMapArray, vec3(v_shadowCoords[lightId].xy / v_shadowCoords[lightId].w + offsetVec, lightId)).r;
	return (shadowDepth < (v_shadowCoords[lightId].z - bias) / v_shadowCoords[lightId].w) ? 0.0 : 1.0;
}

float PCF(in int mode, in int lightId, in vec4 shadowCoordPostW) {
	if ( v_shadowCoords[lightId].w <= 1.0 || (shadowCoordPostW.x > 1.0 || shadowCoordPostW.y > 1.0 || shadowCoordPostW.x < 0.0 || shadowCoordPostW.y < 0.0) )
		return 0.0;

	if (u_enableShadows == 0.0)	return 1.0;

	float x, y;
	float shadow = 0.0;

	switch (mode) {
		case -1:
			shadow = lookupShadowDepth(lightId, vec2(0, 0));

			break;

		// 8x8 kernel PCF
		case 0:
			for (y = -3.5; y <=3.5; y+=1.0)
				for (x = -3.5; x <=3.5; x+=1.0)
					shadow += lookupShadowDepth(lightId, vec2(x,y));
					
			shadow /= 64.0;

			break;

		// 8x8 PCF wide kernel (step is 10 instead of 1)
		case 1:	
			for (y = -30.5; y <= 30.5; y+=10.0)
				for (x = -30.5; x <= 30.5; x+=10.0)
					shadow += lookupShadowDepth(lightId, vec2(x, y));
					
			shadow /= 64.0;

			break;

		// 4x4 kernel PCF
		case 2:		
			for (y = -1.5; y <=1.5; y+=1.0)
				for (x = -1.5; x <=1.5; x+=1.0)
					shadow += lookupShadowDepth(lightId, vec2(x, y));
		
			shadow /= 16.0 ;
		
			break;

		// 4x4  PCF wide kernel (step is 10 instead of 1)
		case 3:
			for (y = -10.5; y <=10.5; y+=10.0)
				for (x = -10.5; x <=10.5; x+=10.0)
					shadow += lookupShadowDepth(lightId, vec2(x, y));
					
			shadow /= 16.0 ;
					
			break;

		// 4x4  PCF dithered
		case 4:			
			// use modulo to vary the sample pattern
			vec2 o = mod(floor(gl_FragCoord.xy), 2.0);
				
			shadow += lookupShadowDepth(lightId, vec2(-1.5, 1.5) + o);
			shadow += lookupShadowDepth(lightId, vec2( 0.5, 1.5) + o);
			shadow += lookupShadowDepth(lightId, vec2(-1.5, -0.5) + o);
			shadow += lookupShadowDepth(lightId, vec2( 0.5, -0.5) + o);
			shadow *= 0.25;

			break;
	}

	return shadow;
}

float calculateShadowVal(int lightId) {
	vec4 shadowCoordPostW = v_shadowCoords[lightId] / v_shadowCoords[lightId].w;

	float shadow = 0.0;
	int shadowMode = -1;	//mod((int) (u_elapsedTime / 2000), 6) - 1;

	vec3 normal = normalize(v_normalW);
	vec3 lightDir = v_posW.xyz - u_lights[lightId].pos;

	float cosTheta = dot(normal, -normalize(lightDir));
	if (acos(cosTheta) < 3.14 / 2.0)
		shadow = PCF(shadowMode, lightId, shadowCoordPostW);


	if (shadow > 0.0) {
		float spotCoeff = abs(shadowCoordPostW.y - 0.5) * abs(shadowCoordPostW.y - 0.5) + abs(shadowCoordPostW.x - 0.5) * abs(shadowCoordPostW.x - 0.5);

		// make it more like a flashlight
		if (spotCoeff < 0.05)
			spotCoeff *= 2.0;

		spotCoeff = 1.0 - spotCoeff * 4.0;

		// reduce the strong edges
		if (spotCoeff < 0.05)
			spotCoeff = 0.0;

		shadow *= spotCoeff;
	}

	return shadow;
}

vec4 calculatePhong(in bool hasNormalMap, in vec4 baseColor, in vec2 textureScale) {
	vec3 normal;
	if (hasNormalMap) {
		vec3 texNormal = texture2D(s_normalHeightTex, v_texcoordL * textureScale).xyz;
    
		mat3 TBN = mat3(normalize(v_tangentW), normalize(v_bitangentW), normalize(v_normalW));
		//mat3 TBN = mat3(normalize(v_tangentW), normalize(cross(v_tangentW, v_normalW)), normalize(v_normalW));
    
		normal = normalize((TBN * (2.0 * texNormal - 1.0)).xyz);
	} else {
		normal = normalize(v_normalW);
	}

	vec4 totalDiffuse = vec4(0);
	vec4 totalSpecular = vec4(0);
	
	for (int i = 0; i < u_numLightSources; i++) {
		if (u_lights[i].type == LIGHTTYPE_DIRECTIONAL)
		{
			vec3 lightDir		= normalize(u_lights[i].pos);
			vec3 reflectVector	= normalize( reflect( lightDir, normal ) );
			
			totalDiffuse		+= max(dot(normal, -lightDir), 0.0) * u_lights[i].diffuse;
			totalSpecular		+= pow( max( dot(reflectVector, normalize(v_viewW)), 0.0 ), 20.0 ) * u_lights[i].specular;
		}
		else if (u_lights[i].type == LIGHTTYPE_POINT || u_lights[i].type == LIGHTTYPE_SPOT)
		{
			vec3 lightDir		= v_posW.xyz - u_lights[i].pos;
		
			float att = 1.0;
			
			float d				=	length(lightDir);
			float onePerAtt		=	(u_lights[i].attenuation.x + 
									(u_lights[i].attenuation.y * d) + 
									(u_lights[i].attenuation.z * d*d));
									
			if (onePerAtt != 0.0)
				att	= 1.00 / onePerAtt;
		
			
			vec3 reflectVector	= normalize( reflect( normalize(lightDir), normal ) );

			if (u_lights[i].type == LIGHTTYPE_SPOT)
				totalDiffuse	+= max(dot(normal, -normalize(lightDir)), 0.0) * u_lights[i].diffuse * 1.0 * att;// * calculateShadowVal(i);
			else
				totalDiffuse	+= max(dot(normal, -normalize(lightDir)), 0.0) * u_lights[i].diffuse * 1.0 * att;


			totalSpecular		+= pow( max( dot(reflectVector, normalize(v_viewW)), 0.0 ), 20.0 ) * u_lights[i].specular;
		}
	}

	float weight = 0.1;
	vec4 finalColor = (u_lights[0].ambient * weight + totalDiffuse * (1.0 - weight)) * baseColor + totalSpecular;

	return finalColor;
}

vec4 calculateLinearFog(in vec4 baseColor) {
	float fogStart = 70.0;
	float fogLength = 200.0;
	vec3 fogColor = vec3(0.2, 0.2, 0.0);

	float distToEye = length(v_viewW);
	
	//float lerpValue = clamp( (distToEye - u_fogStart) / u_fogLength, 0.0, 1.0);
	//vec4 finalColor = lerpValue * vec4(u_fogColor, 1) + (1.0 - lerpValue) * baseColor;

	float lerpValue = clamp( (distToEye - fogStart) / fogLength, 0.0, 1.0);
	
	vec4 finalColor = lerpValue * vec4(fogColor, 1) + (1.0 - lerpValue) * baseColor;
	
	return finalColor;
}

vec2 techniqueBase() {
	return v_texcoordL;
}

float scale	= 0.04;
float bias	= -0.02;

vec2 techniqueParallax() {
	vec2 uv = v_texcoordL;
	vec3 v = normalize(v_viewTBN);

	float height = texture2D(s_normalHeightTex, uv).a * scale + bias;

	uv = height * v.xy + uv;

	return uv;
}

vec2 techniqueBinary() {
	float height;

	vec2 uv = v_texcoordL;
	vec3 v = normalize(v_viewTBN);

	float H, Hmin = 0.0, Hmax = 1.0;
	
	vec2 uvIn = v.xy + uv;
	vec2 uvOut = v_texcoordL;
	
	for (int i = 0; i < 10; i++) {
		H = (Hmin + Hmax) / 2.0;   // middle
		uv = uvIn * H + uvOut * (1.0 - H);
		
		height = texture2D(s_normalHeightTex, uv).a * scale * 0.8 + bias * -0.1 * 0.8;

		if (H <= height) {
			Hmin = H; // below
		} else {
			Hmax = H; // above
		}
	}

	return uv;
}
