#version 440 core

layout(location=0) out vec4 oColor;

in float tHueProgress;

void main() {
	oColor.a = 1.0;
	// Sextant index
	uint si = uint(floor(tHueProgress * 6));
	// Fractional part
	float fr = tHueProgress * 6 - si;
	// Compute non-primary color characteristics
	float pre = 1.0 - fr;
	float post = fr;
	// Switch on sextant
	switch (si % 6)
	{
	case 0:
		oColor.r = 1.0;
		oColor.g = post;
		oColor.b = 0.0;
		break;
	case 1:
		oColor.r = pre;
		oColor.g = 1.0;
		oColor.b = 0.0;
		break;
	case 2:
		oColor.r = 0.0;
		oColor.g = 1.0;
		oColor.b = post;
		break;
	case 3:
		oColor.r = 0.0;
		oColor.g = pre;
		oColor.b = 1.0;
		break;
	case 4:
		oColor.r = post;
		oColor.g = 0.0;
		oColor.b = 1.;
		break;
	case 5:
		oColor.r = 1.0;
		oColor.g = 0.0;
		oColor.b = pre;
		break;
	}
}
