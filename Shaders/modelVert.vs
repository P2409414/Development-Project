
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vNorm;
out vec2 vUV;


void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vNorm = aNorm;
    vUV = aUV;

}