
uniform sampler2D tex;
uniform sampler2D normalHeightMap;

uniform int technique;
uniform float scale;
uniform float bias;

uniform bool enableParallax;

varying vec3 lightDir;
varying vec3 halfVector;
varying vec3 viewDir;

void main() {
	vec2 uv;
	float height;

	vec3 v = normalize(viewDir);
	
	// Simple parallax
	if (technique == 0) {
		vec4 N = texture2D(normalHeightMap, gl_TexCoord[0].st);
		height = N.a * scale + bias;
		uv = height * v.xy + gl_TexCoord[0].st;
		
			
	// Binary search
	} else if (technique == 1) {
		float H, Hmin = 0.0, Hmax = 1.0;
		
		vec2 uvin = v.xy + gl_TexCoord[0].st;
		vec2 uvout = gl_TexCoord[0].st;
		
		for (int i = 0; i < 10; i++) {
			H = (Hmin + Hmax) / 2.0;   // middle
			uv = uvin * H + uvout * (1.0 - H);
			
			//height = texture2D(normalHeightMap, uv).a * scale + bias;
			height = texture2D(normalHeightMap, uv).a * scale * 1.5 + bias * 1.5;
			
			if (H <= height) {
				Hmin = H; // below
			} else {
				Hmax = H; // above
			}
		}
	
	
	// Secant technique
	} else if (technique == 2) {
		float H, Ha = 0.0, Hb = 1.0;
		vec2 uvin = v.xy + gl_TexCoord[0].st;
		vec2 uvout = gl_TexCoord[0].st;

		float D;
		// a csatornaban van a magassag
		float Da = texture2D(normalHeightMap, uvout).a;// * scale + bias;
		float Db = +(texture2D(normalHeightMap, uvin).a) - (scale + bias);// * scale + bias - 1.0;
		
		for (int i = 0; i < 5; i++) {
			H = Hb + (Ha-Hb) / (Da-Db) * Da;
			uv = uvin * H + uvout * (1.0 - H);
			
			D = H - texture2D(normalHeightMap, uv).a;// * scale + bias;
			
			if (D < 0.0) {
				Db = D; Hb = H;
			} else {
				Da = D; Ha = H;
			}
		}
	
		
	// Linear search
	} else if (technique == 3) {
		int LIN_ITER = 4;
				
		vec2 uvin = v.xy + gl_TexCoord[0].st;
		vec2 uvout = gl_TexCoord[0].st;
		
		float H = 1.0; 					// current height
		float Hint = 0.0; 				// height of intersection
		
		float start = scale + bias;
		
		for (int i = 0; i < LIN_ITER; i++) {
			H = start - i * start / LIN_ITER;
			uv = uvin * H + uvout * (1.0 - H);
			
			//height = texture2D(normalHeightMap, uv).a * scale * 1.5 + bias * 1.5;
			height = texture2D(normalHeightMap, uv).a * scale + bias;
			
			if (Hint == 0.0) {			// if no point below yet
				if (H <= height) {
					// no effect with H 
					//Hint = h;			// below
					Hint = H;			// below
				}
			}
		}
		// texture coords of intersection
		uv = uvin * Hint + uvout * (1.0 - Hint);


	// No displacement
	} else {
		uv = gl_TexCoord[0].st;
	}
		
	gl_FragColor = texture2D(tex, uv);// * (height + 0.1) * 5;
}
