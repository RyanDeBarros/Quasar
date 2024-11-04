#version 440 core

layout(location=0) in vec2 i_VertexPosition;
layout(location=1) in float i_TexSlot;
layout(location=2) in vec2 i_UVs;

uniform mat3 u_MVP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

out float t_TexSlot;
out vec2 t_UVs;

void main() {
	t_TexSlot = i_TexSlot;
	t_UVs = i_UVs;
	
	gl_Position.xy = (u_MVP * vec3(i_VertexPosition, 1.0)).xy;
}
