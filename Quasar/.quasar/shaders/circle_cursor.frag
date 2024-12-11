#version 440 core

layout(location=0) out vec4 oColor;

in vec2 tUVs;
in float tInnerRadius;
in float tValue;

void main() {
	float magsq = dot(tUVs, tUVs);
	if (magsq > 1.0 || magsq < tInnerRadius * tInnerRadius) discard;
	oColor = vec4(tValue, tValue, tValue, 1.0);
}
