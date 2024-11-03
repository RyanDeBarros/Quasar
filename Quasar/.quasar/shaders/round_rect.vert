#version 440 core

layout(location=0) in vec2 i_VertexPosition;
layout(location=1) in vec4 i_BorderColor;
layout(location=2) in vec4 i_InteriorColor;
layout(location=3) in vec2 i_RectBottomLeft;
layout(location=4) in vec2 i_RectSize;
layout(location=5) in float i_CornerRadius;
layout(location=6) in float i_Thickness;

uniform mat3 u_VP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

out vec2 t_RelVertexPosition;
out vec4 t_BorderColor;
out vec4 t_InteriorColor;
out vec2 t_RelRectBottomLeft;
out vec2 t_RelRectSize;
out float t_Thickness;

void main() {
	t_RelVertexPosition = i_VertexPosition / i_CornerRadius;
	t_BorderColor = i_BorderColor;
	t_InteriorColor = i_InteriorColor;
	t_RelRectBottomLeft = i_RectBottomLeft / i_CornerRadius;
	t_RelRectSize = i_RectSize / i_CornerRadius;
	t_Thickness = i_Thickness;
	
	gl_Position.xy = (u_VP * vec3(i_VertexPosition, 1.0)).xy;
}
