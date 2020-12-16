#version 330 core
out vec4 FragColor;
in vec3 FragPos;
in vec4 FragPosLightSpace;
in vec2 Texture;
in mat3 TBN;
in vec3 N;

uniform vec3 viewPos;
uniform vec3 LightPos;
uniform vec3 LightColor;
uniform float height_scale;

uniform sampler2D shadowMap;
uniform sampler2D texture;
uniform sampler2D texture_norm;
uniform sampler2D parallaxMap;

vec2 Parallax(vec2 TexCoords, vec3 texViewDir)
{
    float height = texture(parallaxMap, TexCoords).r;
    vec2 p = texViewDir.xy / texViewDir.z * (height * height_scale);
    return TexCoords - p;    
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 Normal)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0)
        return 0.0;
        
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    vec3 lightDir = normalize(LightPos - FragPos);
    float bias = max(0.05 * (1.0 - dot(Normal, lightDir)), 0.005);
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

    shadow /= 9.0;
        
    return shadow;
}

void main()
{
    mat3 trTBN = transpose(TBN);
    vec2 Texture_coordinates = Parallax(Texture, normalize(trTBN * normalize(viewPos - FragPos)));
  // Texture_coordinates = Texture;
    float ambient = 0.3;
    vec3 Normal = texture(texture_norm, Texture_coordinates).rgb;
    // Normal = N;
    Normal = Normal * 2.0 - 1.0;   
    Normal = normalize(TBN * Normal);
    vec3 LightDir = normalize(LightPos - FragPos);  
    vec3 ViewDir = normalize(viewPos - FragPos);
    vec3 HalfwayDir = normalize(LightDir + ViewDir);

    float diff = max(dot(Normal, LightDir), 0.0);
    float specular = 1.0;
    vec3 reflectDir = reflect(-LightDir, Normal);  
    specular *= pow(max(dot(ViewDir, HalfwayDir), 0.0), 64);

    float distance = length(LightPos - FragPos);
    float attenuation = 3.0 / (distance) ;
    diff *= attenuation;
    specular *= attenuation;
    // Вычисляем тень
    float shadow = ShadowCalculation(FragPosLightSpace, Normal);       
    
    vec3 result = (ambient + (1.0 - shadow) * (diff + specular)) * LightColor;    

    FragColor = texture(texture, Texture_coordinates);
    FragColor.x *= result.x;
    FragColor.y *= result.y;
    FragColor.z *= result.z;
    float gamma = 2.2;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
    //if(shadow != 0)
    //{
    //    FragColor.rgb = vec3(0.0, 0.0, 0.0);
    //}
    //FragColor.rgb = vec3(abs(Normal.x), abs(Normal.y), abs(Normal.z));
   // FragColor = texture(texture, Texture_coordinates);
};