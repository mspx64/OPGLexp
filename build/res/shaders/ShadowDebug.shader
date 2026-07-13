#shader Vertex
#version 450 core

layout(location = 0)  in vec3 pos ;
layout(location = 1 ) in vec2 textcoords;

layout(location = 0) out vec2 TexCoord;

void main()
{
    TexCoord = textcoords ;
	gl_Position =  vec4(pos, 1.0);
}

#shader Fragment

#version 450 core
struct Light {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

// Material structure
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float shininess;
    float normalStrength;
    bool hasNormalMap;
    bool hasSpecularMap;
};

// Uniforms
uniform sampler2D u_diffuseMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_specularMap;
uniform sampler2D u_depthMap;   

uniform Material u_material;
uniform Light u_light;
uniform bool u_useColor;
uniform vec3 u_color;

out vec4 FragColor;
  
layout(location = 0) in vec2 TexCoords;

void main()
{             
    float depthValue = texture(u_depthMap, TexCoords).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}