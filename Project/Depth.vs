#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatr;
uniform mat4 model;
 
void main()
{
    gl_Position = lightSpaceMatr * model * vec4(aPos, 1.0);
}