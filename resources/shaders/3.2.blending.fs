#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{             
    vec4 tex = texture(texture1, TexCoords);
    if(tex.a < 0.5)
        discard;
    else
        FragColor = tex * vec4(1.0, 1.0, 1.0, 0.2);
    
}