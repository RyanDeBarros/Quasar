#version 440 core

layout(location=0) out vec4 oColor;

in vec4 tColor;

void main() {
	oColor = tColor;
}
