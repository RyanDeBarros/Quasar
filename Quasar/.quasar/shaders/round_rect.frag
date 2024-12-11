#version 440 core

layout(location=0) out vec4 oColor;

in vec2 tRelVertexPosition;
in vec4 tBorderColor;
in vec4 tInteriorColor;
in vec2 tRelRectBottomLeft;
in vec2 tRelRectSize;
in float tThickness;

void main() {
	vec2 delta = tRelVertexPosition - tRelRectBottomLeft;

	if (delta.x < 1.0) {
		if (delta.y < 1.0) {
			// BL
			vec2 m = 1.0 - delta;
			float magsq = dot(m, m);
			if (magsq > 1.0) discard;
			if (magsq > (1.0 - tThickness) * (1.0 - tThickness)) {
				oColor = tBorderColor;
			} else {
				oColor = tInteriorColor;
			}
		} else if (delta.y > tRelRectSize.y - 1.0) {
			// TL
			vec2 m = vec2(1.0 - delta.x, 1.0 - tRelRectSize.y + delta.y);
			float magsq = dot(m, m);
			if (magsq > 1.0) discard;
			if (magsq > (1.0 - tThickness) * (1.0 - tThickness)) {
				oColor = tBorderColor;
			} else {
				oColor = tInteriorColor;
			}
		} else {
			// Left edge
			if (delta.x < tThickness) {
				oColor = tBorderColor;
			} else {
				oColor = tInteriorColor;
			}
		}
	} else if (delta.x > tRelRectSize.x - 1.0) {
		if (delta.y < 1.0) {
			// BR
			vec2 m = vec2(1.0 - tRelRectSize.x + delta.x, 1.0 - delta.y);
			float magsq = dot(m, m);
			if (magsq > 1.0) discard;
			if (magsq > (1.0 - tThickness) * (1.0 - tThickness)) {
				oColor = tBorderColor;
			} else {
				oColor = tInteriorColor;
			}
		} else if (delta.y > tRelRectSize.y - 1.0) {
			// TR
			vec2 m = vec2(1.0 - tRelRectSize.x + delta.x, 1.0 - tRelRectSize.y + delta.y);
			float magsq = dot(m, m);
			if (magsq > 1.0) discard;
			if (magsq > (1.0 - tThickness) * (1.0 - tThickness)) {
				oColor = tBorderColor;
			} else {
				oColor = tInteriorColor;
			}
		} else {
			// Right edge
			if (delta.x > tRelRectSize.x - tThickness) {
				oColor = tBorderColor;
			} else {
				oColor = tInteriorColor;
			}
		}
	} else {
		if (delta.y < 1.0) {
			// Bottom edge
			if (delta.y < tThickness) {
				oColor = tBorderColor;
			} else {
				oColor = tInteriorColor;
			}
		} else if (delta.y > tRelRectSize.y - 1.0) {
			// Top edge
			if (delta.y > tRelRectSize.y - tThickness) {
				oColor = tBorderColor;
			} else {
				oColor = tInteriorColor;
			}
		} else {
			// Center
			oColor = tInteriorColor;
		}
	}
}
