#version 440 core

layout(location=0) in vec2 iVertexPosition;
layout(location=1) in float iTexSlot;
layout(location=2) in vec2 iUVs;

uniform mat3 uMVP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

out float tTexSlot;
out vec2 tUVs;

void main() {
	tTexSlot = iTexSlot;
	tUVs = iUVs;
	
	gl_Position.xy = (uMVP * vec3(iVertexPosition, 1.0)).xy;
}
