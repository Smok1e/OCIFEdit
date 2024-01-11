#version 430 core

//===========================================

#define GRID_SIZE 10
#define GRID_BRIGHTNESS_EVEN 0.125
#define GRID_BRIGHTNESS_ODD  0.25

uniform vec2 rect_position;
out vec4 out_Color;

//===========================================

void main()
{
	vec2 position = (gl_FragCoord.xy - rect_position * vec2(1, -1));

	out_Color = int((floor(position.x / GRID_SIZE) + floor(position.y / GRID_SIZE))) % 2 == 0
		? vec4(GRID_BRIGHTNESS_EVEN, GRID_BRIGHTNESS_EVEN, GRID_BRIGHTNESS_EVEN, 1)
		: vec4(GRID_BRIGHTNESS_ODD,  GRID_BRIGHTNESS_ODD,  GRID_BRIGHTNESS_ODD,  1);
}

//===========================================