#version 440 core

layout(location=0) out vec4 o_Color;

in vec2 t_UVs;
in float t_Thickness;
in float t_Value;

void main() {
	float mag_sq = dot(t_UVs, t_UVs);
	if (mag_sq > 1.0 || sqrt(mag_sq) < 1.0 - t_Thickness) discard;
	o_Color = vec4(t_Value, t_Value, t_Value, 1.0);
}
