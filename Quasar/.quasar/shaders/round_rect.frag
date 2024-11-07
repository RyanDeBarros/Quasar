#version 440 core

layout(location=0) out vec4 o_Color;

in vec2 t_RelVertexPosition;
in vec4 t_BorderColor;
in vec4 t_InteriorColor;
in vec2 t_RelRectBottomLeft;
in vec2 t_RelRectSize;
in float t_Thickness;

void main() {
	vec2 delta = t_RelVertexPosition - t_RelRectBottomLeft;

	if (delta.x < 1.0) {
		if (delta.y < 1.0) {
			// BL
			vec2 m = 1.0 - delta;
			float mag_sq = dot(m, m);
			if (mag_sq > 1.0) discard;
			if (mag_sq > (1.0 - t_Thickness) * (1.0 - t_Thickness)) {
				o_Color = t_BorderColor;
			} else {
				o_Color = t_InteriorColor;
			}
		} else if (delta.y > t_RelRectSize.y - 1.0) {
			// TL
			vec2 m = vec2(1.0 - delta.x, 1.0 - t_RelRectSize.y + delta.y);
			float mag_sq = dot(m, m);
			if (mag_sq > 1.0) discard;
			if (mag_sq > (1.0 - t_Thickness) * (1.0 - t_Thickness)) {
				o_Color = t_BorderColor;
			} else {
				o_Color = t_InteriorColor;
			}
		} else {
			// Left edge
			if (delta.x < t_Thickness) {
				o_Color = t_BorderColor;
			} else {
				o_Color = t_InteriorColor;
			}
		}
	} else if (delta.x > t_RelRectSize.x - 1.0) {
		if (delta.y < 1.0) {
			// BR
			vec2 m = vec2(1.0 - t_RelRectSize.x + delta.x, 1.0 - delta.y);
			float mag_sq = dot(m, m);
			if (mag_sq > 1.0) discard;
			if (mag_sq > (1.0 - t_Thickness) * (1.0 - t_Thickness)) {
				o_Color = t_BorderColor;
			} else {
				o_Color = t_InteriorColor;
			}
		} else if (delta.y > t_RelRectSize.y - 1.0) {
			// TR
			vec2 m = vec2(1.0 - t_RelRectSize.x + delta.x, 1.0 - t_RelRectSize.y + delta.y);
			float mag_sq = dot(m, m);
			if (mag_sq > 1.0) discard;
			if (mag_sq > (1.0 - t_Thickness) * (1.0 - t_Thickness)) {
				o_Color = t_BorderColor;
			} else {
				o_Color = t_InteriorColor;
			}
		} else {
			// Right edge
			if (delta.x > t_RelRectSize.x - t_Thickness) {
				o_Color = t_BorderColor;
			} else {
				o_Color = t_InteriorColor;
			}
		}
	} else {
		if (delta.y < 1.0) {
			// Bottom edge
			if (delta.y < t_Thickness) {
				o_Color = t_BorderColor;
			} else {
				o_Color = t_InteriorColor;
			}
		} else if (delta.y > t_RelRectSize.y - 1.0) {
			// Top edge
			if (delta.y > t_RelRectSize.y - t_Thickness) {
				o_Color = t_BorderColor;
			} else {
				o_Color = t_InteriorColor;
			}
		} else {
			// Center
			o_Color = t_InteriorColor;
		}
	}
}
