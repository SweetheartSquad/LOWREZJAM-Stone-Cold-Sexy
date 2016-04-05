#version 150

in vec2 Texcoord;

out vec4 outColor;

uniform sampler2D texFramebuffer;
uniform float time = 0;
uniform float whiteout = 0;
uniform float redout = 0;


float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
float rand(float x, float y){
	return rand(vec2(x, y));
}

void main() {
	// get pixels
	vec2 r = Texcoord;
	r *= 64;
	r -= mod(r, 1);
	r /= 64;

	vec2 p = r;
	
	// distort uvs
	//r = vec2(rand(r.x, time), rand(r.y, time));
	r.x += abs(0.5 - r.x) * 0.5;
	r.y += abs(0.5 - r.y) * 0.5;

	r.x += (p.x - r.x) * (1-redout+whiteout);
	r.y += (p.y - r.y) * (1-redout+whiteout);

	// original
	vec4 orig = vec4(texture(texFramebuffer, Texcoord) );

	// new
	vec4 col = vec4(texture(texFramebuffer, r) );
    col.r += (1 - outColor.r)*(redout);
    col.rgb += vec3(whiteout);
	
	// combined
	float mixAmount = max(0., 0.5 - distance(p, vec2(0.5, 0.65))) * (col.g < col.r ? 2 : -1);
	mixAmount += whiteout;

	outColor = mix(orig, col, mixAmount);
	outColor = max(orig, outColor);
	

    //outColor = mix(outColor, vec4(1), whiteout);
}
