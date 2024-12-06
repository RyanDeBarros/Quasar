#version 440 core

layout(points) in;
layout(line_strip, max_vertices = 8) out;

uniform float uOutlineWidth = 2.0;
uniform mat3 uVP;
uniform vec2 uScreenSize;

void main() {
//	vec2 pos = gl_in[0].gl_Position.xy;
//	
//	vec2 ndc = (uOutlineWidth / uScreenSize) * 2.0;
//
//	vec2 corners[4] = vec2[4](
//		pos + vec2(-ndc.x, -ndc.y),
//		pos + vec2(ndc.x, -ndc.y),
//		pos + vec2(-ndc.x, ndc.y),
//		pos + vec2(ndc.x, ndc.y)
//	);
//
//	for (int i = 0; i < 4; ++i) {
//		gl_Position.xy = (uVP * vec3(corners[i], 1.0)).xy;
//		EmitVertex();
//	}
//	EndPrimitive();

	vec2 delta = (uVP * vec3(10000 / uScreenSize.x, 10000 / uScreenSize.y, 1.0)).xy;
	//vec2 delta = (uVP * vec3(10 / uScreenSize.x, 10 / uScreenSize.y, 1.0)).xy;
	//vec2 delta = vec2(100 / uScreenSize.x, 100 / uScreenSize.y);
	
	gl_Position.xy = gl_in[0].gl_Position.xy + delta;
	EmitVertex();
	gl_Position.xy = gl_in[0].gl_Position.xy + vec2(-delta.x, -delta.y);
	EmitVertex();
	gl_Position.xy = gl_in[0].gl_Position.xy + vec2(delta.x, -delta.y);
	EmitVertex();
	EndPrimitive();
}
