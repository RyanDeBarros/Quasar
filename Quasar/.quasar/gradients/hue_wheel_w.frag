#version 440 core

const float TAU = 6.28318531;

layout(location=0) out vec4 o_Color;

in vec2 t_UVs;
in float t_Value;

void main() {
	float mag_sq = dot(t_UVs, t_UVs);
	if (mag_sq > 1.0) discard;

	o_Color.a = 1.0;
	float hue = 0.5 + atan(t_UVs.y, t_UVs.x) / TAU;
	float sat = sqrt(mag_sq);

	// Sextant index
	uint si = uint(floor(6 * hue));

	// Fractional part
	float fr = 6 * hue - si;

	// Compute non-primary color characteristics
	float _min = t_Value * (1.0 - sat);
	float pre = t_Value * (1.0 - sat * fr);
	float post = t_Value * (1.0 - sat * (1.0 - fr));
	
	// Switch on sextant
	switch (si)
	{
	case 0:
		o_Color.r = t_Value;
		o_Color.g = post;
		o_Color.b = _min;
		break;
	case 1:
		o_Color.r = pre;
		o_Color.g = t_Value;
		o_Color.b = _min;
		break;
	case 2:
		o_Color.r = _min;
		o_Color.g = t_Value;
		o_Color.b = post;
		break;
	case 3:
		o_Color.r = _min;
		o_Color.g = pre;
		o_Color.b = t_Value;
		break;
	case 4:
		o_Color.r = post;
		o_Color.g = _min;
		o_Color.b = t_Value;
		break;
	case 5:
		o_Color.r = t_Value;
		o_Color.g = _min;
		o_Color.b = pre;
		break;
	}
}
