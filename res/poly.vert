#version 140

in vec2 position;
uniform vec2 window_halfdim;

void main()
{
	vec2 p = position - window_halfdim;
	p /= window_halfdim;
	gl_Position = vec4(p.xy, 0.0, 1.0);
}
