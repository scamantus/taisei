#version 330 core

#include "lib/render_context.glslh"
#include "interface/reimu_gap.glslh"
#include "lib/util.glslh"

const vec4 gap_color = vec4(0.75, 0, 0.4, 0.9);

void main(void) {
	vec4 view_texel = texture(tex, texCoordGapView);

	vec4 c = vec4(0);
	vec2 uv = texCoord;

	float a = 0.5 * (pi + atan(uv.y - 0.5, uv.x - 0.5));
	a = sin(a*3.0+time) * cos(-a*16.0+2.41*time);
	a = 0.7 * a + 0.3 * sin(-a*8.0+time*1.2);
	a = 0.5 + 0.5 * a;
	a = 1.0 + 0.5 * a * (0.05 + 0.95 * (0.5 - distance(0.5, uv.x)));
	// a *= 1.0 + pow(distance(0.5, uv.y), 1.5);

	float gap = a * distance(uv, vec2(0.5));
	float smoothbase = (1.0 - distance(0.5, uv.x));

	float gap_inner = smoothstep(0.5, 0.5 - smoothbase * 0.2, gap);
	c = alphaCompose(c, view_texel * gap_inner);

	float gap_border = smoothstep(0.5, 0.5 - smoothbase * 0.1, gap) - smoothstep(0.5, 0.5 - smoothbase * 0.5, gap);
	c = alphaCompose(c, gap_color * gap_border * 0.5);

	fragColor = c;
}
