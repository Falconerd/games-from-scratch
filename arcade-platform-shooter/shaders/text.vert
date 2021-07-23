#version 330 core
layout (location = 0) in vec4 vertex;
out vec2 uvs;

uniform mat4 projection;

void main() {
	uvs = vertex.zw;
	gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
}
