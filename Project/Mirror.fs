#version 330 core
out vec4 FragColor;
  
in vec4 FragPosMirSpace;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

uniform sampler2D shadowMap;
uniform sampler2D MirrorTex;
uniform vec3 viewPos;
uniform vec3 LightPos;
uniform vec3 LightColor;


float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0)
        return 0.0;
        
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }

    shadow /= 19.0;
        
    return shadow;
}

void main()
{
    float ambient = 0.3;

    vec3 LightDir = normalize(LightPos - FragPos);  
    vec3 ViewDir = normalize(viewPos - FragPos);
    vec3 HalfwayDir = normalize(LightDir + ViewDir);

    vec3 Norm = normalize(Normal);

    float diff = max(dot(Norm, LightDir), 0.0);
   
    float specular = 2.3;
    vec3 reflectDir = reflect(-LightDir, Norm);  
    specular *= pow(max(dot(ViewDir, HalfwayDir), 0.0), 64);

    float distance = length(LightPos - FragPos);
    float attenuation = 1.0 / (distance * distance) ;
    diff *= attenuation;
    specular *= attenuation;

    float shadow = ShadowCalculation(FragPosLightSpace);       
    
    vec3 result = (ambient + (1.0 - shadow) * (diff + specular)) * LightColor;    

    vec3 projCoords = FragPosMirSpace.xyz / FragPosMirSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    FragColor = texture(MirrorTex, projCoords.xy) * 0.7; 
    FragColor.x *= result.x;
    FragColor.y *= result.y;
    FragColor.z *= result.z;
    float gamma = 2.2;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
};