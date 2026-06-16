#shader Vertex
#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 textcoord;
layout(location = 3) in vec4 tangent;
    

layout(location = 0) out mat3 TBN;
layout(location = 3) out vec2 Textcoord;


// Uniform matrices
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;


void main (){

vec3 T = normalize(mat3(u_Model)*tangent.xyz);
vec3 N = normalize(mat3(u_Model)*normal);

T = normalize(T - dot(T,N) * N);

vec3 B = cross(N, T) * tangent.w;

TBN = mat3(T, B, N);

Textcoord = textcoord;

gl_Position =  u_Projection * u_View * u_Model * vec4(pos, 1.0);

}

#shader Fragment

#version 460 core
#extension GL_ARB_bindless_texture : require


vec3 lightPos = vec3(1.0f, 1.0f, 1.0f);
vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

struct Material {

		vec4 specularColor;
		vec4 diffuseColor;
		vec4 emmisiveCOlor;
		vec4 baseColor;

		float tansperancyFactor;
		float alphaTest;

		sampler2D normalMap;
		sampler2D diffuseMap;
		sampler2D metallicRoughnessMap;

	};

//in
layout(location = 0) in mat3 TBN;
layout(location = 3) in vec2 Textcoord;

//out
layout(location = 0) out vec4 out_color;


//ssbo
layout(std430 , binding = 0) buffer Materials{
	Material  materials[];
};

//uniforms 
uniform int u_MaterialIndex;
uniform int u_DebugMode;

void main(){

	  vec4 basetexture = texture(materials[(u_MaterialIndex)].diffuseMap, Textcoord);
	  vec4 normaltexture = texture(materials[(u_MaterialIndex)].normalMap, Textcoord);

	  vec3 Normal = normalize(TBN * normaltexture.rgb * 2.0 - 1.0);

   out_color = basetexture;
   //out_color = vec4(Textcoord , 1.0 , 1.0);
   //out_color = vec4( normaltexture.rgb * basetexture.rgb, basetexture.a);
   //out_color = vec4(max(dot(lightPos,Normal) , 0.0f)  * basetexture.rgb, basetexture.a);
   //out_color = vec4(max(dot(lightPos,Normal) , 0.0f)*materials[(u_MaterialIndex)].baseColor.rgb, 1.0);
}    
  