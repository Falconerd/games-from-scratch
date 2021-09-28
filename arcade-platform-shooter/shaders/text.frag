#version 330 core
in vec2 uvs;
out vec4 frag_color;

uniform sampler2D tex;
uniform vec4 color;

void main() {
	frag_color = vec4(1.0, 1.0, 1.0, texture(tex, uvs).r) * color;
}

