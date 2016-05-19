#version 140

in vec2 position;
in vec2 tex_coord;
uniform vec2 window_halfdim;
uniform vec2 offset;
out vec2 TexCoord;

void main()
{
    vec2 p = position + offset - window_halfdim;
    p /= window_halfdim;
    gl_Position = vec4(p.xy, 0.0, 1.0);
    TexCoord = tex_coord;
}
