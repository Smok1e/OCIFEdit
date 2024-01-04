#version 450 core

//===========================================

#define GRID_SIZE 10

uniform vec2 rect_position;
out vec4 out_Color;

//===========================================

void main()
{
	vec2 position = (gl_FragCoord.xy - rect_position * vec2(1, -1));

	out_Color = int((floor(position.x / GRID_SIZE) + floor(position.y / GRID_SIZE))) % 2 == 0
		? vec4(0.1, 0.1, 0.1, 1)
		: vec4(0.2, 0.2, 0.2, 1);
}

//===========================================