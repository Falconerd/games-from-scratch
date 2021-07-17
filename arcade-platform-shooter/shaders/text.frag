#version 330 core
out vec4 frag_color;

in vec2 uvs;

uniform vec4 color;
uniform sampler2D texture_id;
uniform float offset;
uniform float width;

void main() {
	float x = 0.0f;
	if (uvs.x == 1.0f) {
		x = width;
	}
	frag_color = texture(texture_id, vec2(x, uvs.y)) * color;
}

