#version 330 core
in vec2 uvs;
out vec4 frag_color;

uniform sampler2D text;
uniform vec4 color;

void main() {
	frag_color = vec4(1, 1, 1, texture(text, uvs).r) * color;
	// frag_color = vec4(1, 1, 1, 1);
}

