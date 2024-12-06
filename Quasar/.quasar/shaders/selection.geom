#version 440 core

layout(points) in;
//layout(triangles) in;
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

	gl_Position = gl_in[0].gl_Position + vec4(0.1, 0.1, 0.0, 0.0);
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + vec4(-0.1, -0.1, 0.0, 0.0);
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + vec4(0.1, -0.1, 0.0, 0.0);
	EmitVertex();
	EndPrimitive();
}
