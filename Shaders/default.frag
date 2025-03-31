#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform sampler2D texture1;
uniform sampler2D texture2;

struct Material {
   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
   float shininess;
};

struct Light {
   vec3 direction;
   vec3 position;
   float cutoff;
   float outerCutOff;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;

   float constant;
   float linear;
   float quadratic;
};

uniform Material material;
uniform Light light;

void main()
{
   vec3 lightDir = normalize(light.position - FragPos);

   float theta = dot(lightDir, normalize(-light.direction));

   if (theta > light.cutoff)
   {

   // Ambient
   vec3 ambient = light.ambient * material.ambient;
     
   // diffuse 
   vec3 norm = normalize(Normal);
   float diff = max(dot(norm, lightDir), 0.0);
   vec3 diffuse = light.diffuse * (diff * material.diffuse);

   // specular
   vec3 viewDir = normalize(viewPos - FragPos);
   vec3 reflectDir = reflect(-lightDir, norm);  
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
   vec3 specular = light.specular * (spec * material.specular);

   // spotlight (soft edges)
   float theta = dot(lightDir, normalize(-light.direction)); 
   float epsilon = (light.cutoff - light.outerCutOff);
   float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
   diffuse  *= intensity;
   specular *= intensity;

   // attenuation
   float distance = length(light.position - FragPos);
   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

   ambient  *= attenuation;  
   diffuse   *= attenuation;
   specular *= attenuation;   
     
   vec3 result = ambient + diffuse + specular;
   FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2) * vec4(result, 1.0);
   }
   else
   {
   vec3 result = light.ambient * material.diffuse;
   FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2) * vec4(result, 1.0);
   }
}
