#version 330 core

in vec2 uv;
out vec4 FragColor;
uniform sampler2D img;

void main() {
	FragColor = texture(img, uv);
}
