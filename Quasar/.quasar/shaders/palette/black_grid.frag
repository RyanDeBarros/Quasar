#version 440 core

layout(location=0) out vec4 o_Color;

uniform float u_ColProportion = 1.0 / 8.0;
uniform float u_RowProportion = 1.0 / 8.0;
uniform float u_Padding = 0.05;
uniform float u_Border = 0.125;

in vec2 t_RelPosition;

void main() {
	o_Color = vec4(0.0, 0.0, 0.0, 1.0);
	float x = mod(t_RelPosition.x, u_ColProportion) / u_ColProportion;
	float y = mod(t_RelPosition.y, u_RowProportion) / u_RowProportion;

	if (x < u_Padding || x > 1.0 - u_Padding)
		discard;
	if (y < u_Padding || y > 1.0 - u_Padding)
		discard;
	if (x > u_Padding + u_Border && x < 1.0 - u_Padding - u_Border && y > u_Padding + u_Border && y < 1.0 - u_Padding - u_Border)
		discard;
}
