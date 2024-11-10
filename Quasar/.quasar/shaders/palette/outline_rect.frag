#version 440 core

layout(location=0) out vec4 o_Color;

uniform float u_Padding = 0.05;
uniform float u_Border = 0.125;

in vec2 t_UVs;
in vec4 t_Color;

void main() {
	o_Color = t_Color;

	if (t_UVs.x < u_Padding || t_UVs.x > 1.0 - u_Padding)
		discard;
	if (t_UVs.y < u_Padding || t_UVs.y > 1.0 - u_Padding)
		discard;
	if (t_UVs.x > u_Padding + u_Border && t_UVs.x < 1.0 - u_Padding - u_Border && t_UVs.y > u_Padding + u_Border && t_UVs.y < 1.0 - u_Padding - u_Border)
		discard;
}
