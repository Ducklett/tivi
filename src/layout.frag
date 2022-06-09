#version 330 core
out vec4 FragColor;
in vec2 uv;
uniform sampler2D img;
uniform vec2 aspect;
void main()
{
	if (aspect.x < aspect.y) {
		float offY = -(aspect.y/aspect.x*.5-.5);
		FragColor = texture(img,vec2(uv.x,offY + uv.y*aspect.y/aspect.x));
	} else {
		float offX = -(aspect.x/aspect.y*.5-.5);
		FragColor = texture(img,vec2(offX + uv.x*aspect.x/aspect.y,uv.y));
	}
}
