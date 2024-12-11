#version 440 core

layout(location=0) in float iTexSlot;
layout(location=1) in vec2 iVertexPosition;
layout(location=2) in vec2 iUV;
layout(location=3) in vec4 iColor;

uniform mat3 uVP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

out vec4 tColor;
out float tTexSlot;
out vec2 tTexCoord;

void main() {
	tColor = iColor;
	tTexSlot = iTexSlot;
	tTexCoord = iUV;
	gl_Position.xy = (uVP * vec3(iVertexPosition, 1.0)).xy;
}
