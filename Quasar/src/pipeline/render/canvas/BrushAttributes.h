#pragma once

#include "edit/color/Color.h"

struct BrushAttributes
{
	struct Tolerance
	{
		float r1 = 0.0f;
		float r2 = 0.0f;
		bool check_r = false;
		bool r_tol(RGBA base, RGBA color) const;
		float g1 = 0.0f;
		float g2 = 0.0f;
		bool check_g = false;
		bool g_tol(RGBA base, RGBA color) const;
		float b1 = 0.0f;
		float b2 = 0.0f;
		bool check_b = false;
		bool b_tol(RGBA base, RGBA color) const;
		float a1 = 0.0f;
		float a2 = 0.0f;
		bool check_a = false;
		bool a_tol(RGBA base, RGBA color) const;
		float h1 = 0.0f;
		float h2 = 0.0f;
		bool check_h = false;
		bool h_tol(HSV base, HSV color) const;
		float s_hsv1 = 0.0f;
		float s_hsv2 = 0.0f;
		bool check_s_hsv = false;
		bool s_hsv_tol(HSV base, HSV color) const;
		float v1 = 0.0f;
		float v2 = 0.0f;
		bool check_v = false;
		bool v_tol(HSV base, HSV color) const;
		float s_hsl1 = 0.0f;
		float s_hsl2 = 0.0f;
		bool check_s_hsl = false;
		bool s_hsl_tol(HSL base, HSL color) const;
		float l1 = 0.0f;
		float l2 = 0.0f;
		bool check_l = false;
		bool l_tol(HSL base, HSL color) const;

		bool tol(RGBA base, RGBA color) const;
	} tolerance;
};
