#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;
 
uniform sampler2D depthMap;
uniform sampler2D shadowMap;

void main()
{             
    float depthValue = texture(depthMap, TexCoords).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}  