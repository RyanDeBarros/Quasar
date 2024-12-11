#version 440 core

layout(location=0) out vec4 oColor;

uniform float uHue = 0.0;

in float tLightnessProgress;

void main() {
	oColor.a = 1.0;
	// Chroma
	float chroma = (1.0 - abs(2.0 * tLightnessProgress - 1.0));
	float x = chroma * (1.0 - abs(mod((6 * uHue), 2.0) - 1.0));
	// RGB channels (unordered)
	float c1 = tLightnessProgress + chroma * 0.5;
	float c2 = tLightnessProgress - chroma * 0.5 + x;
	float c3 = tLightnessProgress - chroma * 0.5;
	// Order RGB channels
	uint si = uint(floor(uHue * 6.0));
	switch (si % 6)
	{
	case 0:
		oColor.r = c1;
		oColor.g = c2;
		oColor.b = c3;
		break;
	case 1:
		oColor.r = c2;
		oColor.g = c1;
		oColor.b = c3;
		break;
	case 2:
		oColor.r = c3;
		oColor.g = c1;
		oColor.b = c2;
		break;
	case 3:
		oColor.r = c3;
		oColor.g = c2;
		oColor.b = c1;
		break;
	case 4:
		oColor.r = c2;
		oColor.g = c3;
		oColor.b = c1;
		break;
	case 5:
		oColor.r = c1;
		oColor.g = c3;
		oColor.b = c2;
		break;
	}
}
