#version 440 core

layout(location=0) in vec2 iPosition;
layout(location=1) in float iLineOrientation;

uniform mat3 uVP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

out float tLineOrientation;

void main() {
	gl_Position.xy = (uVP * vec3(iPosition, 1.0)).xy;
	tLineOrientation = iLineOrientation;
}
