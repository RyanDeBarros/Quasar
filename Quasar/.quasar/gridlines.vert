#version 440 core

layout(location = 0) in vec2 i_VertexPosition;

uniform mat3 u_VP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));
uniform vec4 u_FlatTransform = vec4(0.0, 0.0, 1.0, 1.0);
uniform vec4 u_Color = vec4(0.2, 0.3, 1.0, 1.0);

out vec4 t_Color;

void main() {
	t_Color = u_Color;

	// model matrix
	mat3 M = mat3(vec3(u_FlatTransform[2], 0.0, 0.0), vec3(0.0, u_FlatTransform[3], 0.0), vec3(u_FlatTransform[0], u_FlatTransform[1], 1.0));
	gl_Position.xy = (u_VP * M * vec3(i_VertexPosition, 1.0)).xy;
}
