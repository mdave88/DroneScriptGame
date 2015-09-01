
vec4 calculateLighting(in vec2 uv, in vec4 baseColor) {
	vec4 finalColor;

	if (length(texture2D(s_normalHeightTex, uv).xyz) != 0.0)
		finalColor = calculatePhong(true, baseColor, vec2(1.0));
	else
		finalColor = calculatePhong(false, baseColor, vec2(1.0));

	return finalColor;
}

void main()
{
	vec2 uv;
	int technique = (u_technique < u_techniqueMax) ? u_technique : u_techniqueMax;
	
	if (technique == 0)
		uv = techniqueParallax();
	else if (technique == 1)
		uv = techniqueBinary();
	else
		uv = techniqueBase();

	vec4 finalColor = texture2D(s_colorTex, uv);

	if (finalColor.a < 0.5)
		discard;

	finalColor = calculateLighting(uv, finalColor);
	finalColor = calculateLinearFog(finalColor);

	vec4 lightMapColor = vec4(1.0);
	if (v_lightMapTexcoordL.s + v_lightMapTexcoordL.t != 0.0)
		lightMapColor = vec4(texture2D(s_lightMapTex, v_lightMapTexcoordL).rgb, 1.0) * 2.0;

	gl_FragColor = finalColor * lightMapColor;
}

	//vec2 uv = techniqueBase();

	//float shadow = texture2DArray(u_shadowMapArray, vec3(uv, 1)).r;
	//vec4 finalColor = vec4(vec3(shadow), 1.0);
	//gl_FragColor = finalColor;
