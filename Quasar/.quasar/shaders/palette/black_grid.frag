#version 440 core

layout(location=0) out vec4 oColor;

uniform float uColProportion = 1.0 / 8.0;
uniform float uRowProportion = 1.0 / 8.0;
uniform float uPadding = 0.05;
uniform float uBorder = 0.125;

in vec2 tRelPosition;

void main() {
	oColor = vec4(0.0, 0.0, 0.0, 1.0);
	float x = mod(tRelPosition.x, uColProportion) / uColProportion;
	float y = mod(tRelPosition.y, uRowProportion) / uRowProportion;

	if (x < uPadding || x > 1.0 - uPadding)
		discard;
	if (y < uPadding || y > 1.0 - uPadding)
		discard;
	if (x > uPadding + uBorder && x < 1.0 - uPadding - uBorder && y > uPadding + uBorder && y < 1.0 - uPadding - uBorder)
		discard;
}
