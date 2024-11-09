#version 440 core

layout(location=0) in vec2 i_VertexPosition;

uniform mat3 u_VP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));
uniform vec2 u_RectBottomLeft = vec2(0.0, 0.0);
uniform vec2 u_RectSize = vec2(1.0, 1.0);

out vec2 t_RelPosition;

void main() {
	gl_Position.xy = (u_VP * vec3(i_VertexPosition, 1.0)).xy;

	t_RelPosition = (i_VertexPosition - u_RectBottomLeft) / u_RectSize;
}
