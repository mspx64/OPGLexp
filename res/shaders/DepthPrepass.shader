#shader Vertex
#version 460 core

layout(location = 0) in vec3 pos;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    gl_Position = u_Projection * u_View * u_Model * vec4(pos, 1.0);
}

#shader Fragment
#version 460 core

void main() {
    // Only writing depth, no color output needed.
}
