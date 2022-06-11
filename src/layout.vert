#version 330 core

layout(location = 0) in vec2 aPos;
out vec2 uv;
uniform float scale;
uniform vec2 offset;
uniform vec2 stretch;

void main() {
	uv = aPos / 2 + .5;
	// convert coordinate space (0,0 is top left in images but bottom left in shaders)
	uv.y = 1 - uv.y;

	vec2 pos = aPos;
	pos *= stretch;
	pos += offset;
	pos *= scale;

	gl_Position = vec4(pos.x, pos.y, 0, 1);
}
