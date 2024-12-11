#version 440 core

layout(location=0) in vec2 iVertexPosition;
layout(location=1) in vec2 iUVs;
layout(location=2) in vec4 iGradientColors;

uniform mat3 uVP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

out vec2 tUVs;
out vec4 tGradientColors;

void main() {
	tUVs = iUVs;
	tGradientColors = iGradientColors;
	gl_Position.xy = (uVP * vec3(iVertexPosition, 1.0)).xy;
}
