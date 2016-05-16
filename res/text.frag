#version 140

in vec2 TexCoord;
uniform sampler2D tex;
uniform vec4 color;

void main()
{
    vec4 alpha = texture(tex, TexCoord);
    
    if (alpha.r == 0)
      discard;
    
    gl_FragColor = vec4(1.0, 1.0, 1.0, alpha.r) * color;
}
