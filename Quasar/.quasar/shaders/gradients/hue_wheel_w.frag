#version 440 core

const float TAU = 6.28318531;

layout(location=0) out vec4 oColor;

uniform float uValue = 1.0;

in vec2 tUVs;

void main() {
	float mag_sq = dot(tUVs, tUVs);
	if (mag_sq > 1.0) discard;

	oColor.a = 1.0;
	float hue = -atan(tUVs.y, tUVs.x) / TAU;
	if (hue < 0.0)
		hue += 1.0;
	float sat = sqrt(mag_sq);

	// Sextant index
	uint si = uint(floor(6 * hue));

	// Fractional part
	float fr = 6 * hue - si;

	// Compute non-primary color characteristics
	float _min = uValue * (1.0 - sat);
	float pre = uValue * (1.0 - sat * fr);
	float post = uValue * (1.0 - sat * (1.0 - fr));
	
	// Switch on sextant
	switch (si % 6)
	{
	case 0:
		oColor.r = uValue;
		oColor.g = post;
		oColor.b = _min;
		break;
	case 1:
		oColor.r = pre;
		oColor.g = uValue;
		oColor.b = _min;
		break;
	case 2:
		oColor.r = _min;
		oColor.g = uValue;
		oColor.b = post;
		break;
	case 3:
		oColor.r = _min;
		oColor.g = pre;
		oColor.b = uValue;
		break;
	case 4:
		oColor.r = post;
		oColor.g = _min;
		oColor.b = uValue;
		break;
	case 5:
		oColor.r = uValue;
		oColor.g = _min;
		oColor.b = pre;
		break;
	}
}
