#version 440 core

layout(location=0) in vec2 iVertexPosition;
layout(location=1) in float iHueProgress;

uniform mat3 uVP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

out float tHueProgress;

void main() {
	tHueProgress = iHueProgress;
	gl_Position.xy = (uVP * vec3(iVertexPosition, 1.0)).xy;
}
