#version 440 core

layout(location=0) out vec4 o_Color;

uniform float u_Hue = 0.0; // TODO edit

in float t_LightnessProgress;

void main() {
	o_Color.a = 1.0;
	// Chroma
	float chroma = (1.0 - abs(2.0 * t_LightnessProgress - 1.0));
	float x = chroma * (1.0 - abs(mod((6 * u_Hue), 2.0) - 1.0));
	// RGB channels (unordered)
	float c1 = t_LightnessProgress + chroma * 0.5;
	float c2 = t_LightnessProgress - chroma * 0.5 + x;
	float c3 = t_LightnessProgress - chroma * 0.5;
	// Order RGB channels
	uint si = uint(floor(u_Hue * 6.0));
	switch (si % 6)
	{
	case 0:
		o_Color.r = c1;
		o_Color.g = c2;
		o_Color.b = c3;
		break;
	case 1:
		o_Color.r = c2;
		o_Color.g = c1;
		o_Color.b = c3;
		break;
	case 2:
		o_Color.r = c3;
		o_Color.g = c1;
		o_Color.b = c2;
		break;
	case 3:
		o_Color.r = c3;
		o_Color.g = c2;
		o_Color.b = c1;
		break;
	case 4:
		o_Color.r = c2;
		o_Color.g = c3;
		o_Color.b = c1;
		break;
	case 5:
		o_Color.r = c1;
		o_Color.g = c3;
		o_Color.b = c2;
		break;
	}
}
