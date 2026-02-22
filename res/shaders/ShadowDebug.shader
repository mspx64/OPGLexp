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
out vec4 FragColor;
  
layout(location = 0) in vec2 TexCoords;

uniform sampler2D u_depthMap;

void main()
{             
    float depthValue = texture(u_depthMap, TexCoords).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}