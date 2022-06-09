#version 330 core
layout (location = 0) in vec3 aPos;
out vec2 uv;

uniform float scale;
uniform vec2 offset;

void main()
{
	uv = aPos.xy/2+.5;
	gl_Position = vec4((aPos.x+offset.x)*scale, (aPos.y+offset.y)*scale, 0, 1.0);
}
