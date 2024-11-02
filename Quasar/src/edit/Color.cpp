#include "Color.h"

// LATER use more accurate contrast formulas. perhaps even put some constants in settings.

constexpr float BLACK = 0.0f;
constexpr float WHITE = 1.0f;

float contrast_wb_value_simple_hue(float hue)
{
	return on_interval(hue, 0.1f, 0.45f) ? BLACK : WHITE;
}

float contrast_wb_value_simple_hue_and_sat(float hue, float sat)
{
	return on_interval(hue, 0.1f, 0.45f) || sat < 0.3f ? BLACK : WHITE;
}

float contrast_wb_value_simple_hue_and_value(float hue, float value)
{
	if (on_interval(hue, 0.1f, 0.45f))
		return value < 0.75f ? WHITE : BLACK;
	else
		return WHITE;
}

// LATER use pass-by-value for structs <= 16 bytes
float contrast_wb_value_complex_hsv(HSV hsv)
{
	const float sat1 = 0.0f;
	float hue = hsv.h;
	float sat2 = 1.0f;
	if (on_interval(hue, 0.0f, 1 / 6.0f))
		sat2 = 3.0f * hue + 0.5f;
	else if (on_interval(hue, 1 / 2.0f, 2 / 3.0f))
		sat2 = -3.0f * hue + 2.5f;
	else if (hue > 2 / 3.0f)
		sat2 = 0.5f;

	const float val1 = 0.5f;
	float val2 = 1.0f;
	if (on_interval(hue, 0.0f, 1 / 3.0f))
		val2 = -1.5f * hue + 1.0f;
	else if (on_interval(hue, 1 / 3.0f, 2 / 3.0f))
		val2 = 1.5f * hue;

	float y = (hsv.s - sat1) * (val2 - val1) / (sat2 - sat1) + val1;
	return hsv.v < y ? WHITE : BLACK;
}

float contrast_wb_value_complex_hsva(HSVA hsva)
{
	// TODO
	return hsva.alpha < 0.5f ? WHITE : contrast_wb_value_complex_hsv(hsva.hsv);
}

float contrast_wb_value_simple_hue_and_lightness(float hue, float lightness)
{
	if (on_interval(hue, 0.1f, 0.5f))
		return lightness < 0.4f ? WHITE : BLACK;
	else
		return lightness < 0.8f ? WHITE : BLACK;
}

float contrast_wb_value_complex_hsl(HSL hsl)
{
	if (on_interval(hsl.h, 0.1f, 0.5f))
		return hsl.l < 0.4f ? WHITE : BLACK;
	else
		return hsl.l < 0.8f ? WHITE : BLACK;
}
