#version 440 core

layout(location=0) in vec2 iVertexPosition;
layout(location=1) in vec4 iColor;
layout(location=2) in vec2 iUVs;

uniform mat3 uVP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

out vec2 tUVs;
out vec4 tColor;

void main() {
	tColor = iColor;
	tUVs = iUVs;
	gl_Position.xy = (uVP * vec3(iVertexPosition, 1.0)).xy;
}
