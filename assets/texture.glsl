uniform vec3 iResolution;
// stoy uniform vec3 iChannelResolution[4];
uniform vec3 iChannelResolution;
uniform vec2 iRenderXY;
uniform sampler2D iChannel0;
uniform float iExposure;
uniform float iChromatic;
vec2  fragCoord = gl_FragCoord.xy;

vec4 chromatic( vec2 uv ) {	
	vec2 offset = vec2(iChromatic / 36., .0);
	return vec4(texture(iChannel0, uv + offset.xy).r,  texture(iChannel0, uv).g, texture(iChannel0, uv + offset.yx).b, 1.0);
}
void main() {
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	//vec4 t0 = texture(iChannel0, (uv - iChannelResolution.xy) * iRenderXY.xy);//iChannelResolution.xy
	vec4 t0 = texture(iChannel0, uv);
	vec4 c = vec4(0.0);
	
	//if (iChromatic > 0.0) { t0 = chromatic(uv) * t0; }
	c = t0;c *= iExposure;
   	gl_FragColor = c;
}
