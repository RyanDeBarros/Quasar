#version 440 core

layout(location=0) out vec4 o_Color;

in float t_HueProgress;

void main() {
	o_Color.a = 1.0;
	// Sextant index
	uint si = uint(floor(t_HueProgress * 6));
	// Fractional part
	float fr = t_HueProgress * 6 - si;
	// Compute non-primary color characteristics
	float pre = 1.0 - fr;
	float post = fr;
	// Switch on sextant
	switch (si % 6)
	{
	case 0:
		o_Color.r = 1.0;
		o_Color.g = post;
		o_Color.b = 0.0;
		break;
	case 1:
		o_Color.r = pre;
		o_Color.g = 1.0;
		o_Color.b = 0.0;
		break;
	case 2:
		o_Color.r = 0.0;
		o_Color.g = 1.0;
		o_Color.b = post;
		break;
	case 3:
		o_Color.r = 0.0;
		o_Color.g = pre;
		o_Color.b = 1.0;
		break;
	case 4:
		o_Color.r = post;
		o_Color.g = 0.0;
		o_Color.b = 1.;
		break;
	case 5:
		o_Color.r = 1.0;
		o_Color.g = 0.0;
		o_Color.b = pre;
		break;
	}
}
