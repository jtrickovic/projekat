#version 330 core
out vec4 FragColor;

in vec3 Color;

uniform sampler2D texture1;

void main()
{             
    FragColor = vec4(Color, 1.0) * vec4(1.0, 1.0, 1.0, 0.5);
    
}