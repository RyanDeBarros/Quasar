#version 440 core

layout(location=0) in vec2 iVertexPosition;
layout(location=1) in float iLightnessProgress;

uniform mat3 uVP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

out float tLightnessProgress;

void main() {
	tLightnessProgress = iLightnessProgress;
	gl_Position.xy = (uVP * vec3(iVertexPosition, 1.0)).xy;
}
