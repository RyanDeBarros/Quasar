#version 440 core

layout(location=0) in float i_TexSlot;
layout(location=1) in vec2 i_VertexPosition;
layout(location=2) in vec2 i_UV;
layout(location=3) in vec4 i_Color;

uniform mat3 u_VP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

out vec4 t_Color;
out float t_TexSlot;
out vec2 t_TexCoord;

void main() {
	t_Color = i_Color;
	t_TexSlot = i_TexSlot;
	t_TexCoord = i_UV;

	gl_Position.xy = (u_VP * vec3(i_VertexPosition, 1.0)).xy;
}
