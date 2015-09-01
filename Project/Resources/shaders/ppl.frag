
varying vec4  pos;
varying vec3  norm;


uniform int technique;
uniform int elapsedTime;

// lights
varying vec3 viewDir;
varying vec3 lightDirs[10];
uniform int enabledLights[10];


// textures
uniform sampler2D tex;
uniform sampler2D normalHeightMap;

uniform float scale;
uniform float bias;


// shadow computation
uniform int enableShadows;
uniform sampler2D shadowMap;
uniform sampler2DShadow shadowMapS;

varying vec4 shadowCoord;

uniform float xPixelOffset;
uniform float yPixelOffset;


float lookupShadowDepth(vec2 offSet) {
	return shadow2DProj( shadowMapS, shadowCoord  + vec4(offSet.x * xPixelOffset * shadowCoord.w, offSet.y * yPixelOffset * shadowCoord.w, 0.005, 0.0) ).w;
}

float PCF(int mode, vec4 shadowCoordPostW) {
	if ( shadowCoord.w <= 1.0 || (shadowCoordPostW.x > 1.0 || shadowCoordPostW.y > 1.0 || shadowCoordPostW.x < 0.0 || shadowCoordPostW.y < 0.0) )
		return 0.0;

	float x, y;
	float shadow = 0.0;

	switch (mode) {
		case -1:
			shadow = lookupShadowDepth(vec2(0, 0));

			break;

		// 8x8 kernel PCF
		case 0:
			for (y = -3.5; y <=3.5; y+=1.0)
				for (x = -3.5; x <=3.5; x+=1.0)
					shadow += lookupShadowDepth(vec2(x,y));
					
			shadow /= 64.0;

			break;

		// 8x8 PCF wide kernel (step is 10 instead of 1)
		case 1:	
			for (y = -30.5; y <= 30.5; y+=10.0)
				for (x = -30.5; x <= 30.5; x+=10.0)
					shadow += lookupShadowDepth(vec2(x,y));
					
			shadow /= 64.0;

			break;

		// 4x4 kernel PCF
		case 2:		
			for (y = -1.5; y <=1.5; y+=1.0)
				for (x = -1.5; x <=1.5; x+=1.0)
					shadow += lookupShadowDepth(vec2(x,y));
		
			shadow /= 16.0 ;
		
			break;

		// 4x4  PCF wide kernel (step is 10 instead of 1)
		case 3:
			for (y = -10.5; y <=10.5; y+=10.0)
				for (x = -10.5; x <=10.5; x+=10.0)
					shadow += lookupShadowDepth(vec2(x,y));
					
			shadow /= 16.0 ;
					
			break;

		// 4x4  PCF dithered
		case 4:			
			// use modulo to vary the sample pattern
			vec2 o = mod(floor(gl_FragCoord.xy), 2.0);
				
			shadow += lookupShadowDepth(vec2(-1.5, 1.5) + o);
			shadow += lookupShadowDepth(vec2( 0.5, 1.5) + o);
			shadow += lookupShadowDepth(vec2(-1.5, -0.5) + o);
			shadow += lookupShadowDepth(vec2( 0.5, -0.5) + o);
			shadow *= 0.25;

			break;
	}

	return shadow;
}


float rand(vec2 co) {
	//return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453 * sin(elapsedTime));
}

vec2 techniqueBase() {
	return gl_TexCoord[0].st;
}

vec2 techniqueParallax() {
	vec3 v = normalize(viewDir);
	vec2 uv = gl_TexCoord[0].st;

	float height = texture2D(normalHeightMap, gl_TexCoord[0].st).a * scale + bias;

	uv = height * v.xy + gl_TexCoord[0].st;

	return uv;
}

vec2 techniqueBinary() {
	float height;
	vec3 v = normalize(viewDir);
	vec2 uv = gl_TexCoord[0].st;

	float H, Hmin = 0.0, Hmax = 1.0;
		
	vec2 uvin = v.xy + gl_TexCoord[0].st;
	vec2 uvout = gl_TexCoord[0].st;
		
	for (int i = 0; i < 10; i++) {
		H = (Hmin + Hmax) / 2.0;   // middle
		uv = uvin * H + uvout * (1.0 - H);
		
		height = texture2D(normalHeightMap, uv).a * scale * 1.5 + bias * 1.5;

		if (H <= height) {
			Hmin = H; // below
		} else {
			Hmax = H; // above
		}
	}

	return uv;
}

vec4 applyLight(int i, vec4 frontMatColor, vec2 uv) {
	vec4 finalcolor = vec4(0);

	if (enabledLights[i] > 0) {
		float d  = length(vec3(gl_LightSource[i].position - pos));
		float att = 1.00 / (gl_LightSource[i].constantAttenuation + 
						   (gl_LightSource[i].linearAttenuation * d) + 
						   (gl_LightSource[i].quadraticAttenuation * d*d));

		finalcolor += (gl_LightSource[i].ambient * gl_FrontMaterial.ambient) * att;

		vec3 normal;
		vec3 normData;
		vec3 lightDirection;

		if (length(normData = texture2D(normalHeightMap, uv).xyz) == 0.0) {
			// no bumpmap given -> use the world space vectors
			normal = normalize(norm);
			lightDirection = vec3(gl_LightSource[i].position - pos);
		} else {
			// bumpmap given -> use the tangent space vectors
			normal = normalize(normData * 2.0 - 1.0);
			lightDirection = lightDirs[i];
		}

		float NdotL = max(dot(normal, normalize(lightDirection)), 0.0);

		if (NdotL > 0.0) {
			finalcolor += gl_LightSource[i].diffuse * gl_FrontMaterial.diffuse * NdotL * att;		
		}
	}

	return finalcolor;
}

void main() {
	vec2 uv;

	if (technique == 0) {
		uv = techniqueParallax();
	} else if (technique == 1) {
		uv = techniqueBinary();
	} else {
		uv = techniqueBase();
	}
	
	vec4 finalcolor = (gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient);
	
	if (enableShadows > 0) {
		finalcolor += applyLight(1, finalcolor, uv) * 10.0;

		vec4 shadowCoordPostW = shadowCoord / shadowCoord.w;

		//int shadowMode = mod((int) (elapsedTime / 2000), 6) - 1;
		float shadow = PCF(0, shadowCoordPostW);

		if (shadow > 0.0) {
			float spotCoeff = abs(shadowCoordPostW.y-0.5) * abs(shadowCoordPostW.y-0.5) + abs(shadowCoordPostW.x-0.5) * abs(shadowCoordPostW.x-0.5);

			// make it more like a flashlight
			if (spotCoeff < 0.05)
				spotCoeff *= 2.0;

			spotCoeff = 1.0 - spotCoeff * 4.0;

			// reduce the strong edges
			if (spotCoeff < 0.05)
				spotCoeff = 0.0;

			shadow *= spotCoeff;
		}

		finalcolor *= (shadow);

		for (int i = 0; i < 8; i++)
			finalcolor += applyLight(i, finalcolor, uv);

	} else {
		for (int i = 0; i < 8; i++)
			finalcolor += applyLight(i, finalcolor, uv);
	}
	
	//finalcolor += vec4(0.02, 0.02, 0.02, 1);		// ambient

	gl_FragColor = texture2D(tex, uv) * finalcolor * 2;
	//gl_FragColor = texture2D(normalHeightMap, gl_TexCoord[0].st) * finalcolor;
	//gl_FragColor = texture2D(normalHeightMap, uv) * finalcolor;
	//gl_FragColor = texture2D(normalHeightMap, uv).aaaa * finalcolor;
	//gl_FragColor = vec4(1,1,1,1) * finalcolor;
}
