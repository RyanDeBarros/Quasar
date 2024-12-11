#version 440 core

layout(location=0) in vec2 iVertexPosition;
layout(location=1) in vec4 iBorderColor;
layout(location=2) in vec4 iInteriorColor;
layout(location=3) in vec2 iRectBottomLeft;
layout(location=4) in vec2 iRectSize;
layout(location=5) in float iCornerRadius;
layout(location=6) in float iThickness;

uniform mat3 uVP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

out vec2 tRelVertexPosition;
out vec4 tBorderColor;
out vec4 tInteriorColor;
out vec2 tRelRectBottomLeft;
out vec2 tRelRectSize;
out float tThickness;

void main() {
	tRelVertexPosition = iVertexPosition / iCornerRadius;
	tBorderColor = iBorderColor;
	tInteriorColor = iInteriorColor;
	tRelRectBottomLeft = iRectBottomLeft / iCornerRadius;
	tRelRectSize = iRectSize / iCornerRadius;
	tThickness = iThickness;
	gl_Position.xy = (uVP * vec3(iVertexPosition, 1.0)).xy;
}
