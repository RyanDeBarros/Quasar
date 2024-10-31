#version 440 core

layout(location=0) in vec2 i_VertexPosition;
layout(location=1) in float i_LightnessProgress;

uniform mat3 u_VP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

out float t_LightnessProgress;

void main() {
	t_LightnessProgress = i_LightnessProgress;
	gl_Position.xy = (u_VP * vec3(i_VertexPosition, 1.0)).xy;
}
