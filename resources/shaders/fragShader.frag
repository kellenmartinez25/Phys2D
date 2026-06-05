#version 330 core
in vec2 pos;
out vec4 FragColor;
uniform vec3 spriteColor;
uniform bool drawBorder;
uniform vec2 size;
void main() 
{
	vec2 thickness = vec2(10) / size;
	bool border = 
		pos.x <= thickness.x ||
		pos.x >= 1.0 - thickness.x ||
		pos.y <= thickness.y ||
		pos.y >= 1.0 - thickness.y;
	border = border || !drawBorder;
	if (border)
		FragColor = vec4(spriteColor, 1.0);
	else
		discard;
}