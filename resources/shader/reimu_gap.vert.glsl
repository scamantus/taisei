#version 330 core

#include "lib/render_context.glslh"
#include "lib/util.glslh"
#include "interface/reimu_gap.glslh"

vec2 calc_view_uv(vec2 uv, vec2 surface_size, vec2 view_pos) {
	vec2 view_uv = uv;
	view_uv *= surface_size;
	vec2 o = vec2(view_pos.x - surface_size.x * 0.5, (viewport.y - view_pos.y) - surface_size.y * 0.5);
	view_uv += o;
	view_uv /= viewport;
	return view_uv;
}

void main(void) {
	float x = gapCoords.x;
	float y = gapCoords.y;
	float w = gap_size.x;
	float h = gap_size.y;

	vec2 view = gapCoords.zw;
	float rot_angle = gapAngles.x;
	float view_angle = gapAngles.y;

	float c = cos(rot_angle);
	float s = sin(rot_angle);

	mat4 m_rot = mat4(
		 c, s, 0, 0,
		-s, c, 0, 0,
		 0, 0, 1, 0,
		 0, 0, 0, 1
	);

	mat4 m_scale = mat4(
		 w, 0, 0, 0,
		 0, h, 0, 0,
		 0, 0, 1, 0,
		 0, 0, 0, 1
	);

	mat4 m_trans = mat4(
		 1, 0, 0, 0,
		 0, 1, 0, 0,
		 0, 0, 1, 0,
		 x, y, 0, 1
	);

	mat4 mv = m_trans * m_rot * m_scale;

	gl_Position = r_projectionMatrix * mv * vec4(position, 1.0);
	texCoord = texCoordRawIn;
	texCoordGapView = calc_view_uv(texCoord, vec2(w, h), view);

	vec2 view_center = calc_view_uv(vec2(0.5), vec2(w, h), view);
	float aspect = (viewport.x/viewport.y);

	texCoordGapView -= view_center;
	texCoordGapView.x *= aspect;
	texCoordGapView *= rot(-view_angle);
	texCoordGapView.x /= aspect;
	texCoordGapView += view_center;
}
