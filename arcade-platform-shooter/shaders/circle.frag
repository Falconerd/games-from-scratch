#version 460 core

out vec4 FragColor;

uniform vec4 color;
uniform vec2 position;
uniform float radius;

void main() {
	// Start at the pixel offset by the position minus radius divided by the diameter.
	vec2 st = vec2(gl_FragCoord.x - position.x + radius, gl_FragCoord.y - position.y + radius)/vec2(radius * 2);
	float pct = distance(st, vec2(0.5));
        if (pct > 0.5) {
		FragColor = vec4(0);
        } else {
		FragColor = color;
        }
}

