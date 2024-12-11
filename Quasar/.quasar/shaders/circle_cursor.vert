#version 440 core

layout(location=0) in vec2 iVertexPosition;
layout(location=1) in vec2 iUVs;
layout(location=2) in float iThickness;
layout(location=3) in float iValue;

uniform mat3 uVP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

out vec2 tUVs;
out float tInnerRadius;
out float tValue;

void main() {
	tUVs = 2.0 * iUVs - vec2(1.0);
	tInnerRadius = 1.0 - iThickness;
	tValue = iValue;
	gl_Position.xy = (uVP * vec3(iVertexPosition, 1.0)).xy;
}
