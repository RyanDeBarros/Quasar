#include "BrushAttributes.h"

bool BrushAttributes::Tolerance::r_tol(RGBA base, RGBA color) const
{
	return on_interval(color.rgb.r, base.rgb.r - r1, base.rgb.r + r2);
}

bool BrushAttributes::Tolerance::g_tol(RGBA base, RGBA color) const
{
	return on_interval(color.rgb.g, base.rgb.g - g1, base.rgb.g + g2);
}

bool BrushAttributes::Tolerance::b_tol(RGBA base, RGBA color) const
{
	return on_interval(color.rgb.b, base.rgb.b - b1, base.rgb.b + b2);
}

bool BrushAttributes::Tolerance::a_tol(RGBA base, RGBA color) const
{
	return on_interval(color.alpha, base.alpha - a1, base.alpha + a2);
}

bool BrushAttributes::Tolerance::h_tol(HSV base, HSV color) const
{
	float bh1 = base.h - h1;
	float bh2 = base.h + h2;
	if (bh1 < 0.0f)
	{
		if (bh2 > 1.0f)
			return true;
		else
			return color.h <= bh2 || color.h >= bh1 + 1.0f;
	}
	else
	{
		if (bh2 > 1.0f)
			return color.h >= bh1 || color.h <= bh2 - 1.0f;
		else
			return on_interval(color.h, bh1, bh2);
	}
}

bool BrushAttributes::Tolerance::s_hsv_tol(HSV base, HSV color) const
{
	return on_interval(color.s, base.s - s_hsv1, base.s + s_hsv2);
}

bool BrushAttributes::Tolerance::v_tol(HSV base, HSV color) const
{
	return on_interval(color.v, base.v - v1, base.v + v2);
}

bool BrushAttributes::Tolerance::s_hsl_tol(HSL base, HSL color) const
{
	return on_interval(color.s, base.s - s_hsl1, base.s + s_hsl2);
}

bool BrushAttributes::Tolerance::l_tol(HSL base, HSL color) const
{
	return on_interval(color.l, base.l - l1, base.l + l2);
}

bool BrushAttributes::Tolerance::tol(RGBA base, RGBA color) const
{
	bool tolerate = false;
	if (check_r)
	{
		if (!r_tol(base, color))
			return false;
		tolerate = true;
	}
	if (check_g)
	{
		if (!g_tol(base, color))
			return false;
		tolerate = true;
	}
	if (check_b)
	{
		if (!b_tol(base, color))
			return false;
		tolerate = true;
	}
	if (check_a)
	{
		if (!a_tol(base, color))
			return false;
		tolerate = true;
	}
	HSV base_hsv = base.to_hsva().hsv;
	HSV color_hsv = color.to_hsva().hsv;
	if (check_h)
	{
		if (!h_tol(base_hsv, color_hsv))
			return false;
		tolerate = true;
	}
	if (check_s_hsv)
	{
		if (!s_hsv_tol(base_hsv, color_hsv))
			return false;
		tolerate = true;
	}
	if (check_v)
	{
		if (!v_tol(base_hsv, color_hsv))
			return false;
		tolerate = true;
	}
	HSL base_hsl = base.to_hsla().hsl;
	HSL color_hsl = color.to_hsla().hsl;
	if (check_s_hsl)
	{
		if (!s_hsl_tol(base_hsl, color_hsl))
			return false;
		tolerate = true;
	}
	if (check_l)
	{
		if (!l_tol(base_hsl, color_hsl))
			return false;
		tolerate = true;
	}
	return tolerate || base == color;
}
