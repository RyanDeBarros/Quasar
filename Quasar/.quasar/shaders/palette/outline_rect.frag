#version 440 core

layout(location=0) out vec4 oColor;

uniform float uPadding = 0.05;
uniform float uBorder = 0.105;

in vec2 tUVs;
in vec4 tColor;

void main() {
	oColor = tColor;

	if (tUVs.x < uPadding || tUVs.x > 1.0 - uPadding)
		discard;
	if (tUVs.y < uPadding || tUVs.y > 1.0 - uPadding)
		discard;
	if (tUVs.x > uPadding + uBorder && tUVs.x < 1.0 - uPadding - uBorder && tUVs.y > uPadding + uBorder && tUVs.y < 1.0 - uPadding - uBorder)
		discard;
}
