#version 440 core

layout(location = 0) in vec2 iVertexPosition;

uniform mat3 uVP = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));
uniform vec4 uFlatTransform = vec4(0.0, 0.0, 1.0, 1.0);
uniform vec4 uColor = vec4(0.2, 0.3, 1.0, 1.0);

out vec4 tColor;

void main() {
	tColor = uColor;

	// model matrix
	mat3 M = mat3(vec3(uFlatTransform[2], 0.0, 0.0), vec3(0.0, uFlatTransform[3], 0.0), vec3(uFlatTransform[0], uFlatTransform[1], 1.0));
	gl_Position.xy = (uVP * M * vec3(iVertexPosition, 1.0)).xy;
}
