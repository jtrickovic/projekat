#version 330 core

layout (location = 0) in vec3 aPos;       
layout (location = 1) in vec3 aColor;     
layout (location = 2) in vec3 aTranslate; 

out vec3 Color;

uniform mat4 view;        
uniform mat4 projection; 

void main()
{
    
    mat4 model = mat4(1.0f);
    
    model[3][0] = aTranslate.x;
    model[3][1] = aTranslate.y;
    model[3][2] = aTranslate.z;
    
    float scale = 0.25f; 
    mat4 scalingMatrix = mat4(1.0f); 
    scalingMatrix[0][0] = scale;
    scalingMatrix[1][1] = scale;
    scalingMatrix[2][2] = scale;
    
    model = model * scalingMatrix;
    
    Color = aColor;

    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}