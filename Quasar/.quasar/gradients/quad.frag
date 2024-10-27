#version 440 core

#define MAX_GRADIENT_COLORS 4

uniform vec4 u_GradientColors[MAX_GRADIENT_COLORS];

layout(location=0) out vec4 o_Color;

in vec2 t_UVs;
in vec4 t_GradientColors;

void main() {
	vec4 low = mix(u_GradientColors[int(t_GradientColors[0])], u_GradientColors[int(t_GradientColors[1])], t_UVs.x);
	vec4 high = mix(u_GradientColors[int(t_GradientColors[2])], u_GradientColors[int(t_GradientColors[3])], t_UVs.x);
	o_Color = mix(low, high, t_UVs.y);
}
