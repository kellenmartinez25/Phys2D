#version 330 core
layout (location = 0) in vec2 vertex;
out vec2 pos;
uniform mat4 model;
uniform mat4 projection;

void main() {
	pos = vertex;
	gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}