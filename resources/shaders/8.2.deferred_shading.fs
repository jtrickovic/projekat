#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float Radius;       
};

struct DirLight {
    vec3 direction;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;
};


struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};
const int NR_LIGHTS = 64;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;
uniform SpotLight spotlights[NR_LIGHTS];
uniform float exposure;
float intensity = 1.0;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 Diffuse, float Specular) {
    //ambient
    vec3 ambient = light.ambient * Diffuse;
    //diffuse
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = light.diffuse * diff * Diffuse;
    //specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0f;

    // Blinn-Phong
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir),0.0), 16.0);

    vec3 specular = light.specular * spec * Specular;

    return (ambient + diffuse + specular);
}


vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 Diffuse, float Specular)
{
    
    float distance = length(light.position - fragPos);
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * Diffuse * light.color;
            
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    vec3 specular = light.color * spec * Specular;
            
    float attenuation = 1.0 / (1.0 + light.linear * distance + light.quadratic * distance * distance);
    
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
            
    
    return (diffuse + specular);
}

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // calculate distance between light source and current fragment
        float distance = length(lights[i].Position - FragPos);
        if(distance < lights[i].Radius)
        {
            // diffuse
            vec3 lightDir = normalize(lights[i].Position - FragPos);
            vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;
            // specular
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
            vec3 specular = lights[i].Color * spec * Specular;
            // attenuation
            float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
            //float attenuation = 1.0 / (distance * distance);
            diffuse *= attenuation;
            specular *= attenuation;
            
            lighting += diffuse + specular;
        }
    }
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
       float distance = length(spotlights[i].position - FragPos);
       if(distance < spotlights[i].Radius)
           lighting += CalcSpotLight(spotlights[i], Normal, FragPos, viewDir, Diffuse, Specular);
    }

    DirLight light;
    light.diffuse = vec3(0.1f, 0.1f,0.1f);
    light.specular = vec3(0.1f, 0.1f,0.1f);
    light.ambient = vec3(0.1f, 0.1f,0.1f);
    light.direction = vec3(0.0f,-1.0f,0.0f);
    lighting += CalcDirLight(light, Normal, viewDir, Diffuse, Specular);
    
    vec3 result = lighting;
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.8)
    {
        BrightColor = vec4(result, 1.0);
        //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
    else
    {
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
        
    }
    FragColor = vec4(lighting, 1.0);
}