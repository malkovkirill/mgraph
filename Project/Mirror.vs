#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aNormal;
 
out vec4 FragPosMirSpace;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;


uniform mat4 MirSpaceMatr;
uniform mat4 LightSpaceMatr;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
 
void main()
{

    vec3 FragPos = vec3(model * vec4(aPos, 1.0f));
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    FragPosMirSpace = MirSpaceMatr * vec4(FragPos, 1.0f);
    FragPosLightSpace = LightSpaceMatr * vec4(FragPos, 1.0f);
    Normal = transpose(inverse(mat3(model))) * aNormal;
}  