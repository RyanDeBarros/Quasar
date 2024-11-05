#version 440 core

layout(location=0) in vec2 i_VertexPosition;
layout(location=1) in vec2 i_UVs;

uniform mat3 u_VP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

out vec2 t_UVs;

void main() {
	t_UVs = 2.0 * i_UVs - vec2(1.0);
	gl_Position.xy = (u_VP * vec3(i_VertexPosition, 1.0)).xy;
}